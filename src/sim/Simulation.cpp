//
//  Simulation.cpp
//  crag
//
//  Created by John on 10/19/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Simulation.h"

#include "Entity.h"
#include "Firmament.h"
#include "gravity.h"
#include "EntityFunctions.h"
#include "EntitySet.h"

#include "physics/Engine.h"

#include "form/FormationManager.h"

#include "script/ScriptThread.h"

#include "gfx/Renderer.h"

#include "core/app.h"
#include "core/ConfigEntry.h"


CONFIG_DEFINE (sim_tick_duration, Time, 1.f / 60.f);


namespace
{
	CONFIG_DEFINE (apply_gravity, bool, true);
}


////////////////////////////////////////////////////////////////////////////////
// sim::Simulation


using namespace sim;


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

void Simulation::OnAddObject(Entity & entity)
{
	// Until the UpdateModels call is complete, 
	// the data sent to the Renderer is in an incomplete state.
	gfx::Daemon::Call(& gfx::Renderer::OnSetReady, false);
	
	_entity_set.Add(entity);
	
	gfx::Daemon::Call(& gfx::Renderer::OnSetTime, _time);
	gfx::Daemon::Call(& gfx::Renderer::OnSetReady, true);
}

void Simulation::OnRemoveObject(Uid const & uid)
{
	Entity * entity = _entity_set.GetEntity(uid);
	if (entity == nullptr)
	{
		ASSERT(false);
		return;
	}
	
	_entity_set.Remove(* entity);
	
	// TODO: This code is sensitive to object-lifetime issues. More reason for UIDs.
	entity->~Entity();
	Free(entity);
}

void Simulation::OnAttachEntities(Uid const & uid1, Uid const & uid2)
{
	AttachEntities(uid1, uid2, _entity_set, _physics_engine);
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

Entity * Simulation::GetObject(Uid uid) 
{
	ASSERT(uid);
	Entity * entity = _entity_set.GetEntity(uid);
	return entity;
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
	FirmamentHandle skybox;
	skybox.Create<void *>(nullptr);
	gfx::Daemon::Call(& gfx::Renderer::OnSetParent, skybox.GetUid(), gfx::Uid());
	
	Time next_tick_time = app::GetTime();
	
	while (! quit_flag)
	{
		message_queue.DispatchMessages(* this);
		
		Time time = app::GetTime();
		Time time_to_next_tick = next_tick_time - time;
		if (time_to_next_tick > 0)
		{
			smp::Sleep(time_to_next_tick);
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
				next_tick_time += sim_tick_duration;
			}
		}
	}

	// remove skybox
	skybox.Destroy();
}

void Simulation::Tick()
{
	// Tick the entities.
	if (! paused) 
	{
		_time += sim_tick_duration;
		
		// Update the script thread's time variable.
		script::Daemon::Call<Time>(& script::ScriptThread::SetTime, _time);
		
		// Perform the Entity-specific simulation.
		TickEntities();
		
		if (apply_gravity)
		{
			ApplyGravity(_entity_set, sim_tick_duration);
		}

		// Run physics/collisions.
		_physics_engine.Tick(sim_tick_duration);

		_entity_set.Purge();
		
		// Tell renderer about changes.
		UpdateRenderer();
	}
}

void Simulation::UpdateRenderer() const
{
	gfx::Daemon::Call(& gfx::Renderer::OnSetReady, false);
	
	UpdateModels(_entity_set);
	
	gfx::Daemon::Call(& gfx::Renderer::OnSetTime, _time);
	gfx::Daemon::Call(& gfx::Renderer::OnSetReady, true);
}

// Perform a step in the simulation. 
void Simulation::TickEntities()
{
	Entity::List & entities = _entity_set.GetEntities();
	for (Entity::List::iterator it = entities.begin(), end = entities.end(); it != end; ++ it)
	{
		Entity & entity = * it;
		
		entity.Tick(* this);
	}
}
