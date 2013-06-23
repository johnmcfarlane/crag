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

#include "gfx/axes.h"
#include "gfx/Engine.h"
#include "gfx/SetCameraEvent.h"
#include "gfx/SetOriginEvent.h"

#include "core/app.h"
#include "core/ConfigEntry.h"
#include "core/Roster.h"

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
, _origin(geom::abs::Vector3::Zero())
, _physics_engine(ref(new physics::Engine))
, _tick_roster(ref(new core::locality::Roster))
, _draw_roster(ref(new core::locality::Roster))
{
}

Engine::~Engine()
{
	delete & _tick_roster;
	delete & _physics_engine;
}

void Engine::OnQuit()
{
	quit_flag = true;
}

// TODO: Really need this?
void Engine::OnAddObject(Entity &)
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

void Engine::operator() (gfx::SetCameraEvent const & event)
{
	_camera = gfx::GetCameraRay(event.transformation);
}

geom::rel::Ray3 const & Engine::GetCamera() const
{
	return _camera;
}

void Engine::operator() (gfx::SetOriginEvent const & event)
{
	// figure out the delta
	geom::rel::Vector3 delta = geom::Cast<geom::rel::Scalar>(event.origin - _origin);

	// quit if there's no change
	if (geom::LengthSq(delta) == 0)
	{
		return;
	}
	
	ForEachObject([& delta] (Entity & entity) {
		ResetOrigin(entity, delta);
	});
	
	// TODO: Is there a risk of a render between calls to SetOrigin and Draw?
	// If so, could it result in a bad frame?
	UpdateRenderer();

	// local collision formation scene
	auto & scene = _physics_engine.GetScene();
	scene.OnOriginReset(event.origin);

	_origin = event.origin;
}

geom::abs::Vector3 const & Engine::GetOrigin() const
{
	return _origin;
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

core::locality::Roster & Engine::GetTickRoster()
{
	return _tick_roster;
}

core::locality::Roster & Engine::GetDrawRoster()
{
	return _draw_roster;
}

void Engine::Run(Daemon::MessageQueue & message_queue)
{
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

	// stop listening for SetCameraEvent
	ipc::Listener<Engine, gfx::SetCameraEvent>::SetIsListening(false);
	ipc::Listener<Engine, gfx::SetOriginEvent>::SetIsListening(false);

	gfx::Daemon::Call([] (gfx::Engine & engine) {
		engine.OnSetTime(std::numeric_limits<core::Time>::max());
	});
}

void Engine::Tick()
{
	// Tick the entities.
	if (paused) 
	{
		return;
	}

	_time += sim_tick_duration;

	// tick everything
	_tick_roster.Call();

	// Perform the Entity-specific simulation.
	PurgeEntities();

	if (apply_gravity)
	{
		ApplyGravity(* this, sim_tick_duration);
	}

	// Run physics/collisions.
	_physics_engine.Tick(sim_tick_duration, _camera);

	// Tell renderer about changes.
	UpdateRenderer();
}

void Engine::UpdateRenderer() const
{
	STAT_SET(sim_origin, GetOrigin());

	gfx::Daemon::Call([] (gfx::Engine & engine) {
		engine.OnSetReady(false);
	});
	
	_draw_roster.Call();

	auto time = _time;
	gfx::Daemon::Call([time] (gfx::Engine & engine) {
		engine.OnSetTime(time);
		engine.OnSetReady(true);
	});
}

// Perform a step in the simulation. 
void Engine::PurgeEntities()
{
	std::vector<Entity *> to_delete;

	ForEachObject([& to_delete] (Entity & entity) {
		physics::Location const * location = entity.GetLocation();
		if (location == nullptr)
		{
			return;
		}
		
		Vector3 position = location->GetPosition();
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
