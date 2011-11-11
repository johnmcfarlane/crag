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
#include "Entity.h"
#include "Firmament.h"
#include "gravity.h"
#include "EntitySet.h"

#include "physics/Engine.h"

#include "form/FormationManager.h"

#include "script/ScriptThread.h"

#include "gfx/Renderer.h"

#include "core/ConfigEntry.h"


////////////////////////////////////////////////////////////////////////////////
// sim::Simulation


using namespace sim;


CONFIG_DEFINE_MEMBER (Simulation, target_frame_seconds, Time, 1.f / 60.f);
CONFIG_DEFINE_MEMBER (Simulation, apply_gravity, bool, true);


Simulation::Simulation()
: quit_flag(false)
, paused(false)
, _time(0)
, _entity_set(ref(new EntitySet))
, _physics_engine(ref(new physics::Engine))
{
}

Simulation::~Simulation()
{
	delete & _entity_set;
	delete & _physics_engine;
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
	
	_entity_set.Add(* entity);
	entity->UpdateModels();
}

void Simulation::OnRemoveEntity(Uid const & uid)
{
	Entity * entity = _entity_set.GetEntity(uid);
	if (entity == nullptr)
	{
		Assert(false);
		return;
	}
	
	_entity_set.Remove(* entity);
	
	// TODO: This code is sensitive to object-lifetime issues. More reason for UIDs.
	entity->~Entity();
	Free(entity);
}

void Simulation::OnAttachEntities(Uid const & uid1, Uid const & uid2)
{
	Uid uids[2] = { uid1, uid2 };
	physics::Body * bodies[2];
	
	for (int index = 0; index < 2; ++ index)
	{
		Entity * entity = _entity_set.GetEntity(uids[index]);
		if (entity == nullptr)
		{
			return;
		}

		physics::Body * body = entity->GetBody();
		if (body == nullptr)
		{
			return;
		}
		
		bodies[index] = body;
	}
	
	_physics_engine.Attach(ref(bodies[0]), ref(bodies[1]));
}

void Simulation::OnTogglePause()
{
	paused = ! paused;
}

void Simulation::OnToggleGravity()
{
	apply_gravity = ! apply_gravity;
}

void Simulation::OnToggleCollision()
{
	_physics_engine.ToggleCollisions();
}

Time Simulation::GetTime() const
{
	return _time;
}

EntitySet & Simulation::GetEntities() 
{
	return _entity_set;
}

physics::Engine & Simulation::GetPhysicsEngine()
{
	return _physics_engine;
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
		TickEntities();
		
		if (apply_gravity)
		{
			Entity::List & entities = _entity_set.GetEntities();
			ApplyGravity(entities, target_frame_seconds);
		}

		// Run physics/collisions.
		_physics_engine.Tick(target_frame_seconds);

		_entity_set.Purge();
		
		// Tell renderer about changes.
		UpdateRenderer();
	}
}

void Simulation::UpdateRenderer() const
{
	gfx::Daemon::Call(false, & gfx::Renderer::OnSetReady);
	
	UpdateModels();
	
	gfx::Daemon::Call(true, & gfx::Renderer::OnSetReady);
}

// Perform a step in the simulation. 
void Simulation::TickEntities()
{
	Entity::List & entities = _entity_set.GetEntities();
	for (Entity::List::iterator it = entities.begin(), end = entities.end(); it != end; ++ it)
	{
		Entity & e = * it;
		e.Tick(* this);
	}
}

void Simulation::UpdateModels() const
{
	Entity::List const & entities = _entity_set.GetEntities();
	for (Entity::List::const_iterator it = entities.begin(), end = entities.end(); it != end; ++ it)
	{
		Entity const & e = * it;
		e.UpdateModels();
	}
}
