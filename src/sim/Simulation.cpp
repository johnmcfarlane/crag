/*
 *  Simulation.cpp
 *  Crag
 *
 *  Created by John on 10/19/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Simulation.h"

#include "axes.h"
#include "Firmament.h"
#include "Entity.h"
#include "Universe.h"

#include "physics/Engine.h"

#include "form/FormationManager.h"

#include "script/ScriptThread.h"

#include "gfx/Renderer.h"

#include "core/ConfigEntry.h"


////////////////////////////////////////////////////////////////////////////////
// sim::Simulation


using namespace sim;


CONFIG_DEFINE_MEMBER (Simulation, target_frame_seconds, Time, 1.f / 60.f);

Simulation::Simulation()
: quit_flag(false)
, paused(false)
, _time(0)
, universe(new Universe)
, physics_engine(new physics::Engine)
{
}

Simulation::~Simulation()
{
	delete universe;
	universe = nullptr;

	delete physics_engine;
	physics_engine = nullptr;
}

void Simulation::OnQuit()
{
	quit_flag = true;
}

void Simulation::OnAddEntity(Entity * const & entity, PyObject * const & args)
{
	bool initialized = entity->Init(* this, * args);

	// Let go of arguments object.
	// (Incremented in script::MetaClass::NewObject.)
	Py_DECREF(args);
	
	if (! initialized)
	{
		std::cerr << "Bad input parameters to entity" << std::endl;
		DEBUG_BREAK();
		return;
	}
	
	universe->AddEntity(* entity);
	entity->UpdateModels();
}

void Simulation::OnRemoveEntity(Entity * const & entity)
{
	universe->RemoveEntity(* entity);
	
	// TODO: This code is sensitive to object-lifetime issues. More reason for UIDs.
	entity->~Entity();
	Free(entity);
}

void Simulation::OnTogglePause()
{
	paused = ! paused;
}

void Simulation::OnToggleGravity()
{
	universe->ToggleGravity();
}

void Simulation::OnToggleCollision()
{
	physics_engine->ToggleCollisions();
}

Time Simulation::GetTime() const
{
	return _time;
}

Universe & Simulation::GetUniverse() 
{
	return ref(universe);
}

physics::Engine & Simulation::GetPhysicsEngine()
{
	return ref(physics_engine);
}

void Simulation::Run(Daemon::MessageQueue & message_queue)
{
	FUNCTION_NO_REENTRY;
	
	smp::SetThreadPriority(0);
	smp::SetThreadName("Simulation");

	// Add the skybox.
	Firmament * skybox = new Firmament;
	gfx::Daemon::Call<gfx::Object *>(skybox, & gfx::Renderer::OnAddObject);
	
	sys::Time next_tick_time = sys::GetTime();
	
	while (! quit_flag)
	{
		message_queue.DispatchMessages(* this);
		
		sys::Time time = sys::GetTime();
		sys::Time time_to_next_tick = next_tick_time - time;
		if (time_to_next_tick > 0)
		{
			smp::Yield();
		}
		else
		{
			Tick();
			
			if (time_to_next_tick < -1)
			{
				next_tick_time = time;
			}
			else
			{
				next_tick_time += target_frame_seconds;
			}
		}
	}

	// remove skybox
	gfx::Daemon::Call<gfx::Object *>(skybox, & gfx::Renderer::OnRemoveObject);
}

void Simulation::Tick()
{
	// Tick the entities.
	if (! paused) 
	{
		_time += target_frame_seconds;
		
		// Perform the Entity-specific simulation.
		universe->Tick(* this);

		// Run physics/collisions.
		physics_engine->Tick(target_frame_seconds);

		universe->Purge();
		
		// Tell renderer about changes.
		UpdateRenderer();
	}
}

void Simulation::UpdateRenderer() const
{
	gfx::Daemon::Call(false, & gfx::Renderer::OnSetReady);
	
	universe->UpdateModels();
	
	gfx::Daemon::Call(true, & gfx::Renderer::OnSetReady);
}

