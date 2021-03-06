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

#if defined(CRAG_SIM_FORMATION_PHYSICS)
#include "form/Engine.h"
#endif

#include "physics/Location.h"
#include "physics/Engine.h"

#include "gfx/axes.h"
#include "gfx/Engine.h"
#include "gfx/SetCameraEvent.h"
#include "gfx/SetSpaceEvent.h"

#include "core/ConfigEntry.h"
#include "core/Roster.h"

CONFIG_DEFINE(sim_tick_duration, 1. / 60.);
CONFIG_DECLARE(profile_mode, bool);

namespace
{
	CONFIG_DEFINE(apply_gravity, true);
	CONFIG_DEFINE(purge_distance, 1000000000000.);

	STAT_DEFAULT(sim_space, geom::uni::Vector3, 0.3f, geom::uni::Vector3::Zero());
#if defined(CRAG_SIM_FORMATION_PHYSICS)
	STAT(form_changed_sim, bool, 0);
#endif
}


////////////////////////////////////////////////////////////////////////////////
// sim::Engine


using namespace sim;


Engine::Engine()
: quit_flag(false)
, _time(0)
, _camera(Ray3::Zero())
, _lod_parameters({ Vector3::Zero(), 1.f })
, _physics_engine(new physics::Engine)
#if defined(CRAG_SIM_FORMATION_PHYSICS)
, _collision_scene(new form::Scene(512, 512))
#endif
{
}

Engine::~Engine()
{
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Engine, self)
	CRAG_VERIFY_OP(self._pause_counter, >=, 0);
	CRAG_VERIFY(static_cast<super const &>(self));
CRAG_VERIFY_INVARIANTS_DEFINE_END

void Engine::OnQuit()
{
	quit_flag = true;
}

// TODO: Really need this?
void Engine::OnAddObject(ObjectSharedPtr const &)
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

#if defined(CRAG_SIM_FORMATION_PHYSICS)
void Engine::AddFormation(form::Formation & formation)
{
	_collision_scene->AddFormation(formation, GetSpace());
}

void Engine::RemoveFormation(form::Formation & formation)
{
	_collision_scene->RemoveFormation(formation);
}
#else
void Engine::AddFormation(form::Formation &)
{
}

void Engine::RemoveFormation(form::Formation &)
{
}
#endif

void Engine::operator() (gfx::SetCameraEvent const & event)
{
	auto camera_ray = gfx::GetCameraRay(event.transformation);
	_camera = _space.AbsToRel(camera_ray);
}

Ray3 const & Engine::GetCamera() const
{
	return _camera;
}

void Engine::operator() (gfx::SetSpaceEvent const & event)
{
	// figure out the delta
	auto delta = static_cast<Vector3>(event.space - _space);

	// quit if there's no change
	if (geom::MagnitudeSq(delta) == 0)
	{
		return;
	}
	
	ForEachObject([& delta] (Entity & entity) {
		ResetSpace(entity, delta);
	});
	
	// TODO: Is there a risk of a render between calls to SetSpace and Draw?
	// If so, could it result in a bad frame?
	UpdateRenderer();

#if defined(CRAG_SIM_FORMATION_PHYSICS)
	// local collision formation scene
	_collision_scene->OnSpaceReset(event.space, _lod_parameters);
#endif

	// camera
	_camera = geom::Convert(_camera, _space, event.space);
	
	// LOD parameters
	_lod_parameters.center = geom::Convert(_lod_parameters.center, _space, event.space);

	_space = event.space;
}

geom::Space const & Engine::GetSpace() const
{
	return _space;
}

void Engine::operator() (gfx::SetLodParametersEvent const & event)
{
	_lod_parameters = event.parameters;
}

gfx::LodParameters const & Engine::GetLodParameters() const
{
	return _lod_parameters;
}

void Engine::IncrementPause(int increment)
{
	CRAG_VERIFY(* this);

	_pause_counter += increment;

	CRAG_VERIFY(* this);
}

bool Engine::IsPaused() const
{
	CRAG_VERIFY(* this);
	
	return _pause_counter > 0;
}

void Engine::OnToggleGravity()
{
	apply_gravity = ! apply_gravity;
}

void Engine::OnToggleCollision()
{
	_physics_engine->ToggleCollisions();
}

void Engine::OnToggleFormationSuspended()
{
	_collision_scene->SetPaused(! _collision_scene->IsPaused());
}

core::Time Engine::GetTime() const
{
	return _time;
}

std::uint64_t Engine::GetNumTicks() const
{
	return _num_ticks;
}

physics::Engine & Engine::GetPhysicsEngine()
{
	return * _physics_engine;
}

#if defined(CRAG_SIM_FORMATION_PHYSICS)
form::Scene & Engine::GetScene()
{
	return * _collision_scene;
}

form::Scene const & Engine::GetScene() const
{
	return * _collision_scene;
}
#endif

bool Engine::IsSettled() const
{
#if defined(CRAG_SIM_FORMATION_PHYSICS)
	if (quit_flag)
	{
		return true;
	}
	
	return _collision_scene->IsSettled();
#else
	return true;
#endif
}

crag::core::Roster & Engine::GetTickRoster()
{
	static crag::core::Roster _tick_roster;	// general simulation tick
	return _tick_roster;
}

crag::core::Roster & Engine::GetDrawRoster()
{
	static crag::core::Roster _draw_roster;	// provides opportunity to communicate graphical changes to renderer
	return _draw_roster;
}

void Engine::Run(Daemon::MessageQueue & message_queue)
{
	auto next_tick_time = app::GetTime();

	while (! quit_flag)
	{
		if (IsPaused()) 
		{
			message_queue.DispatchMessage(* this);
			continue;
		}

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

			if (! profile_mode)
			{
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

	// stop listening for SetCameraEvent
	ipc::Listener<Engine, gfx::SetCameraEvent>::SetIsListening(false);
	ipc::Listener<Engine, gfx::SetSpaceEvent>::SetIsListening(false);
	ipc::Listener<Engine, gfx::SetLodParametersEvent>::SetIsListening(false);

	gfx::Daemon::Call([] (gfx::Engine & engine) {
		engine.OnSetTime(std::numeric_limits<core::Time>::max());
	});
}

void Engine::Tick()
{
#if defined(CRAG_SIM_FORMATION_PHYSICS)
	if (! _collision_scene->IsPaused() && _collision_scene->Tick(_lod_parameters))
		STAT_SET(form_changed_sim, true);
	else
		STAT_SET(form_changed_sim, false);
#endif

	// tick everything
	GetTickRoster().Call();

	// Perform the Entity-specific simulation.
	PurgeEntities();

	if (apply_gravity)
	{
		ApplyGravity(* this, sim_tick_duration);
	}

	// Run physics/collisions.
	_physics_engine->Tick(sim_tick_duration);

	// Tell renderer about changes.
	UpdateRenderer();

	_time += sim_tick_duration;
	++ _num_ticks;
}

void Engine::UpdateRenderer() const
{
	STAT_SET(sim_space, GetSpace().RelToAbs(Vector3::Zero()));

	gfx::Daemon::Call([] (gfx::Engine & engine) {
		engine.OnSetReady(false);
	});

	GetDrawRoster().Call();

	auto time = _time;
	gfx::Daemon::Call([time] (gfx::Engine & engine) {
		engine.OnSetTime(time);
		engine.OnSetReady(true);
	});
}

// Perform a step in the simulation. 
void Engine::PurgeEntities()
{
	ForEachObject_ReleaseIf([] (Entity & entity) {
		auto const & location = entity.GetLocation();
		if (! location)
		{
			return false;
		}
		
		Vector3 position = location->GetTranslation();
		if (Magnitude(position) < purge_distance)
		{
			return false;
		}
		
		DEBUG_MESSAGE("Removing entity with bad position, %f,%f,%f", position.x, position.y, position.z);
		return true;
	});
}
