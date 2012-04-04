//
//  Engine.cpp
//  crag
//
//  Created by John on 10/19/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Engine.h"

#include "Entity.h"
#include "Firmament.h"
#include "gravity.h"
#include "EntityFunctions.h"
#include "EntitySet.h"

#include "physics/Engine.h"

#include "form/Engine.h"

#include "script/Engine.h"

#include "gfx/Renderer.h"

#include "core/app.h"
#include "core/ConfigEntry.h"


CONFIG_DEFINE (sim_tick_duration, Time, 1.f / 60.f);


namespace
{
	CONFIG_DEFINE (apply_gravity, bool, true);
}


////////////////////////////////////////////////////////////////////////////////
// sim::Engine


using namespace sim;


Engine::Engine()
: quit_flag(false)
, paused(false)
, _time(0)
, _entity_set(ref(new EntitySet))
, _physics_engine(ref(new physics::Engine))
{
}

Engine::~Engine()
{
	delete & _entity_set;
	delete & _physics_engine;
}

void Engine::OnQuit()
{
	quit_flag = true;
}

void Engine::OnAddObject(Entity & entity)
{
	// Until the UpdateModels call is complete, 
	// the data sent to the Renderer is in an incomplete state.
	gfx::Daemon::Call(& gfx::Renderer::OnSetReady, false);
	
	_entity_set.Add(entity);
	
	gfx::Daemon::Call(& gfx::Renderer::OnSetTime, _time);
	gfx::Daemon::Call(& gfx::Renderer::OnSetReady, true);
}

void Engine::OnRemoveObject(Uid const & uid)
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

void Engine::OnAttachEntities(Uid const & uid1, Uid const & uid2)
{
	AttachEntities(uid1, uid2, _entity_set, _physics_engine);
}

void Engine::OnTogglePause()
{
	paused = ! paused;
}

void Engine::OnToggleGravity()
{
	apply_gravity = ! apply_gravity;
}

void Engine::OnToggleCollision()
{
	_physics_engine.ToggleCollisions();
}

Time Engine::GetTime() const
{
	return _time;
}

Entity * Engine::GetObject(Uid uid) 
{
	ASSERT(uid);
	Entity * entity = _entity_set.GetEntity(uid);
	return entity;
}

physics::Engine & Engine::GetPhysicsEngine()
{
	return _physics_engine;
}

void Engine::Run(Daemon::MessageQueue & message_queue)
{
	FUNCTION_NO_REENTRY;
	
	smp::SetThreadPriority(0);

	// Add the skybox.
	FirmamentHandle skybox;
	skybox.Create();
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

void Engine::Tick()
{
	// Tick the entities.
	if (! paused) 
	{
		_time += sim_tick_duration;
		
		// Update the script thread's time variable.
		script::Daemon::Call<Time>(& script::Engine::SetTime, _time);
		
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

void Engine::UpdateRenderer() const
{
	gfx::Daemon::Call(& gfx::Renderer::OnSetReady, false);
	
	UpdateModels(_entity_set);
	
	gfx::Daemon::Call(& gfx::Renderer::OnSetTime, _time);
	gfx::Daemon::Call(& gfx::Renderer::OnSetReady, true);
}

// Perform a step in the simulation. 
void Engine::TickEntities()
{
	Entity::List & entities = _entity_set.GetEntities();
	for (Entity::List::iterator it = entities.begin(), end = entities.end(); it != end; ++ it)
	{
		Entity & entity = * it;
		
		entity.Tick(* this);
	}
}
