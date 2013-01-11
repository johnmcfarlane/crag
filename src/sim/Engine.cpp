//
//  sim/Engine.cpp
//  crag
//
//  Created by John on 10/19/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Engine.h"

#include "gravity.h"
#include "Entity.h"
#include "EntityFunctions.h"

#include "form/Engine.h"

#include "physics/Body.h"
#include "physics/Engine.h"

#include "gfx/Engine.h"

#include "core/app.h"
#include "core/ConfigEntry.h"


CONFIG_DEFINE (sim_tick_duration, core::Time, 1.f / 60.f);


namespace
{
	CONFIG_DEFINE (apply_gravity, bool, true);

	// TODO: This could cause the Observer to be destroyed
	CONFIG_DEFINE(purge_distance, double, 1000000000000.);

	STAT_DEFAULT(sim_origin, geom::abs::Vector3, 0.3f, geom::abs::Vector3::Zero());
}


////////////////////////////////////////////////////////////////////////////////
// sim::Engine


using namespace sim;


Engine::Engine()
: quit_flag(false)
, paused(false)
, _time(0)
, _camera(geom::rel::Ray3::Zero())
, _physics_engine(ref(new physics::Engine))
{
}

Engine::~Engine()
{
	delete & _physics_engine;
}

void Engine::OnQuit()
{
	quit_flag = true;
}

// TODO: Really need this?
void Engine::OnAddObject(Entity & entity)
{
	// Until the UpdateModels call is complete, 
	// the data sent to the gfx::Engine is in an incomplete state.
	gfx::Daemon::Call([] (gfx::Engine & engine) {
		engine.OnSetReady(false);
	});
	
	auto time = _time;
	gfx::Daemon::Call([time] (gfx::Engine & engine) {
		engine.OnSetTime(time);
		engine.OnSetReady(true);
	});
}

void Engine::OnAttachEntities(Uid uid1, Uid uid2)
{
	auto& entity1 = ref(GetObject(uid1));
	auto& entity2 = ref(GetObject(uid2));
	AttachEntities(entity1, entity2, _physics_engine);
}

void Engine::AddFormation(form::Formation& formation)
{
	form::Daemon::Call([& formation] (form::Engine & engine) {
		engine.OnAddFormation(formation);
	});

	auto& scene = _physics_engine.GetScene();
	scene.AddFormation(formation, GetOrigin());
}

void Engine::RemoveFormation(form::Formation& formation)
{
	form::Daemon::Call([& formation] (form::Engine & engine) {
		engine.OnRemoveFormation(formation);
	});

	auto& scene = _physics_engine.GetScene();
	scene.RemoveFormation(formation);
}

void Engine::SetCamera(geom::rel::Ray3 const & camera)
{
	if (_camera == camera)
	{
		return;
	}

	_camera = camera;

	// update form
	form::Daemon::Call([camera] (form::Engine & engine) {
		engine.SetCamera(camera);
	});
}

geom::rel::Ray3 const & Engine::GetCamera() const
{
	return _camera;
}

void Engine::OnSetOrigin(geom::abs::Vector3 const & origin)
{
	// figure out the delta
	auto& scene = _physics_engine.GetScene();
	auto& previous_origin = GetOrigin();
	geom::rel::Vector3 delta = geom::Cast<geom::rel::Scalar>(origin - previous_origin);

	// quit if there's no change
	if (geom::LengthSq(delta) == 0)
	{
		return;
	}
	
	// formation engine
	form::Daemon::Call([origin] (form::Engine & engine) {
		engine.SetOrigin(origin);
	});
	
	ForEachObject([& delta] (Entity & entity) {
		ResetOrigin(entity, delta);
	});
	
	// render engine
	gfx::Daemon::Call([origin] (gfx::Engine & engine) {
		engine.SetOrigin(origin);
	});

	gfx::Daemon::Call([] (gfx::Engine & engine) {
		engine.OnSetReady(false);
	});
	
	ForEachObject([] (Entity const & entity) {
		entity.UpdateModels();
	});
	
	gfx::Daemon::Call([] (gfx::Engine & engine) {
		engine.OnSetReady(true);
	});

	// local collision formation scene
	scene.OnOriginReset(origin);
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

core::Time Engine::GetTime() const
{
	return _time;
}

physics::Engine & Engine::GetPhysicsEngine()
{
	return _physics_engine;
}

void Engine::Run(Daemon::MessageQueue & message_queue)
{
	FUNCTION_NO_REENTRY;
	
	core::Time next_tick_time = app::GetTime();
	
	while (! quit_flag)
	{
		message_queue.DispatchMessages(* this);
		
		core::Time time = app::GetTime();
		core::Time time_to_next_tick = next_tick_time - time;
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
}

void Engine::Tick()
{
	// Tick the entities.
	if (! paused) 
	{
		_time += sim_tick_duration;
		
		// Perform the Entity-specific simulation.
		TickEntities();
		
		if (apply_gravity)
		{
			ApplyGravity(* this, sim_tick_duration);
		}

		// Run physics/collisions.
		VerifyObject(* this);
		_physics_engine.Tick(sim_tick_duration, _camera);

		// Tell renderer about changes.
		UpdateRenderer();
	}
}

void Engine::UpdateRenderer() const
{
	STAT_SET(sim_origin, GetOrigin());

	gfx::Daemon::Call([] (gfx::Engine & engine) {
		engine.OnSetReady(false);
	});
	
	ForEachObject([] (Entity const & entity) {
		entity.UpdateModels();
	});

	auto time = _time;
	gfx::Daemon::Call([time] (gfx::Engine & engine) {
		engine.OnSetTime(time);
		engine.OnSetReady(true);
	});
}

// Perform a step in the simulation. 
void Engine::TickEntities()
{
	std::vector<Entity *> to_delete;

	ForEachObject([& to_delete] (Entity & entity) {
		entity.Tick();

		physics::Body const * body = entity.GetBody();
		if (body == nullptr)
		{
			return;
		}
		
		Vector3 position = body->GetPosition();
		if (Length(position) < purge_distance)
		{
			return;
		}
		
		DEBUG_MESSAGE("Removing entity with bad position, %f,%f,%f", position.x, position.y, position.z);

		to_delete.push_back(& entity);
	});

	for (auto entity : to_delete)
	{
		DestroyObject(entity->GetUid());
	}
}
