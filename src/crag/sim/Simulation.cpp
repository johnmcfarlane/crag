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

CONFIG_DEFINE_MEMBER (sim::Simulation, target_frame_seconds, double, 1.f / 60.f);

sim::Simulation::Simulation()
: quit_flag(false)
, paused(false)
, universe(new Universe)
, physics_engine(new physics::Engine)
{
}

sim::Simulation::~Simulation()
{
	delete universe;
	universe = nullptr;

	delete physics_engine;
	physics_engine = nullptr;
}

void sim::Simulation::OnMessage(smp::TerminateMessage const & message)
{
	quit_flag = true;
}

void sim::Simulation::OnMessage(AddEntityMessage const & message)
{
	Entity & entity = message.entity;

	bool initialized = entity.Init(* this, message.args);

	// Let go of arguments object.
	// (Incremented in script::MetaClass::NewObject.)
	Py_DECREF(& message.args);
	
	if (! initialized)
	{
		std::cerr << "Bad input parameters to entity" << std::endl;
		DEBUG_BREAK();
		return;
	}
	
	universe->AddEntity(entity);
	entity.UpdateModels();
}

void sim::Simulation::OnMessage(RemoveEntityMessage const & message)
{
	universe->RemoveEntity(message.entity);
	
	// TODO: This code is sensitive to object-lifetime issues. More reason for UIDs.
	message.entity.~Entity();
	Free(& message.entity);
}

void sim::Simulation::OnMessage(TogglePauseMessage const & message)
{
	paused = ! paused;
}

void sim::Simulation::OnMessage(ToggleGravityMessage const & message)
{
	universe->ToggleGravity();
}

void sim::Simulation::OnMessage(ToggleCollisionMessage const & message)
{
	physics_engine->ToggleCollisions();
}

void sim::Simulation::OnMessage(gfx::RendererReadyMessage const & message)
{
	Assert(false);
}

sys::TimeType sim::Simulation::GetTime() const
{
	return universe->GetTime();
}

sim::Universe & sim::Simulation::GetUniverse() 
{
	return ref(universe);
}

physics::Engine & sim::Simulation::GetPhysicsEngine()
{
	return ref(physics_engine);
}

void sim::Simulation::Run(Daemon::MessageQueue & message_queue)
{
	FUNCTION_NO_REENTRY;
	
	smp::SetThreadPriority(0);
	smp::SetThreadName("Simulation");

	gfx::AddObjectMessage add_skybox_message = { ref(new Firmament) };
	gfx::Renderer::Daemon::SendMessage(add_skybox_message);
	
	sys::TimeType next_tick_time = sys::GetTime();
	
	while (! quit_flag)
	{
		message_queue.DispatchMessages(* this);
		
		sys::TimeType time = sys::GetTime();
		sys::TimeType time_to_next_tick = next_tick_time - time;
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
	
	gfx::RemoveObjectMessage remove_skybox_message = { add_skybox_message._object };
	gfx::Renderer::Daemon::SendMessage(remove_skybox_message);
}

void sim::Simulation::Tick()
{
	// Tick the entities.
	if (! paused) 
	{
		// Perform the Entity-specific simulation.
		universe->Tick(* this, target_frame_seconds);

		// Run physics/collisions.
		physics_engine->Tick(target_frame_seconds);		
		
		// Tell renderer about changes.
		UpdateRenderer();
	}
}

void sim::Simulation::UpdateRenderer() const
{
	gfx::RenderReadyMessage message;
	message.ready = false;
	gfx::Renderer::Daemon::SendMessage(message);
	
	universe->UpdateModels();
	
	message.ready = true;
	gfx::Renderer::Daemon::SendMessage(message);
}

