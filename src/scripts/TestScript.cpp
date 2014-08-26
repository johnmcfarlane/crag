//
//  TestScript.cpp
//  crag
//
//  Created by John McFarlane on 2012-02-09.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "InitSpace.h"

#include "entity/SpawnEntityFunctions.h"
#include "entity/SpawnPlayer.h"
#include "entity/SpawnSkybox.h"

#include "applet/Applet.h"
#include "applet/AppletInterface_Impl.h"

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "physics/Location.h"

#include "gfx/axes.h"
#include "gfx/Color.h"
#include "gfx/Engine.h"
#include "gfx/object/Object.h"

#include "core/ConfigEntry.h"
#include "core/EventWatcher.h"
#include "core/Random.h"

using geom::Vector3f;
using applet::AppletInterface;

CONFIG_DECLARE(origin_dynamic_enable, bool);

namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// types
	typedef std::vector<sim::EntityHandle> EntityVector;
		
	////////////////////////////////////////////////////////////////////////////////
	// setup variables
	
	geom::abs::Vector3 observer_start_pos(0, 9999400, -5);
	size_t max_shapes = 50;
	bool cleanup_shapes = true;
	CONFIG_DEFINE(spawn_vehicle, bool, true);
	CONFIG_DEFINE(vehicle_thrust, sim::Scalar, 15.f);
	CONFIG_DEFINE(spawn_planets, bool, true);
	
	////////////////////////////////////////////////////////////////////////////////
	// variables

	Random random_sequence;
	sim::EntityHandle _vehicle;
	EntityVector _shapes;
	core::EventWatcher _event_watcher;
	bool _enable_dynamic_origin = true;
	
	////////////////////////////////////////////////////////////////////////////////
	// functions
	
	// random number generation
	double GetRandomUnit()
	{
		return random_sequence.GetUnit<double>();
	}
	
	void SpawnShapes(int shape_num, AppletInterface & applet_interface)
	{
		if (max_shapes == 0)
		{
			return;
		}
	
		if (cleanup_shapes)
		{
			if (_shapes.size() >= max_shapes)
			{
				EntityVector::iterator first = _shapes.begin();
				first->Release();
				_shapes.erase(first);
			}
		}
	
		if (_shapes.size() >= max_shapes)
		{
			return;
		}

		auto camera_ray = applet_interface.Get<sim::Engine, sim::Ray3>([] (sim::Engine & engine) -> sim::Ray3 {
			return engine.GetCamera();
		});
	
		sim::Vector3 spawn_pos = geom::Project(camera_ray, sim::Scalar(5));
	
		gfx::Color4f color(Random::sequence.GetUnitInclusive<float>(), 
					Random::sequence.GetUnitInclusive<float>(), 
					Random::sequence.GetUnitInclusive<float>());

		switch (shape_num)
		{
			case 0:
			{
				// ball
				float radius = geom::rel::Scalar(std::exp(- GetRandomUnit() * 2));
				sim::Sphere3 sphere(spawn_pos, radius);
				sim::EntityHandle ball = SpawnBall(sphere, sim::Vector3::Zero(), color);
				_shapes.push_back(ball);
				break;
			}
			
			case 1:
			{
				// box
				sim::Vector3 size(geom::rel::Scalar(std::exp(GetRandomUnit() * -1.)),
					geom::rel::Scalar(std::exp(GetRandomUnit() * -1.)),
					geom::rel::Scalar(std::exp(GetRandomUnit() * -1.)));
				sim::EntityHandle box = SpawnBox(spawn_pos, sim::Vector3::Zero(), size, color);
				_shapes.push_back(box);
				break;
			}
			
			default:
				ASSERT(false);
		}
	}

	// returns true if the applet should NOT quit
	void HandleEvents(AppletInterface & applet_interface)
	{
		int num_events = 0;
	
		SDL_Event event;
		while (_event_watcher.PopEvent(event))
		{
			++ num_events;
		
			if (event.type != SDL_KEYDOWN)
			{
				continue;
			}

			switch (event.key.keysym.scancode)
			{
				case SDL_SCANCODE_ESCAPE:
				{
					app::Quit();
					break;
				}

				case SDL_SCANCODE_I:
					if ((event.key.keysym.mod & KMOD_CTRL) == 0)
					{
						break;
					}
				
					if ((event.key.keysym.mod & (KMOD_SHIFT | KMOD_ALT | KMOD_CTRL)) != 0)
					{
						break;
					}
				
					_enable_dynamic_origin = ! _enable_dynamic_origin;
					break;
				
				case SDL_SCANCODE_COMMA:
					SpawnShapes(0, applet_interface);
					break;
				
				case SDL_SCANCODE_PERIOD:
					SpawnShapes(1, applet_interface);
					break;
				
				default:
					break;
			}
		}
	}
}

// main entry point
void TestScript(AppletInterface & applet_interface)
{
	FUNCTION_NO_REENTRY;

	// Create sun. 
	geom::abs::Sphere3 star_volume(geom::abs::Vector3(65062512., 75939904., 0.), 1000000.);
	gfx::Color4f star_color(gfx::Color4f(1.f,.95f,.85f) * 8000000000000000.f);
	sim::EntityHandle sun1 = SpawnStar(star_volume, star_color);
	sim::EntityHandle sun2 = SpawnStar(star_volume, gfx::Color4f::Yellow() * 1000000000000000.f);
	
	// Create planets
	sim::EntityHandle planet, moon1, moon2;
	if (spawn_planets)
	{
		sim::Scalar planet_radius = 10000000;

		planet = SpawnPlanet(sim::Sphere3(sim::Vector3::Zero(), planet_radius), 3634, 0);
		moon1 = SpawnPlanet(sim::Sphere3(sim::Vector3(planet_radius * 1.5f, planet_radius * 2.5f, planet_radius * 1.f), 1500000), 10, 250);
		moon2 = SpawnPlanet(sim::Sphere3(sim::Vector3(planet_radius * -2.5f, planet_radius * 0.5f, planet_radius * -1.f), 2500000), 13, 0);
	}
	
	auto const & origin = observer_start_pos;
	InitSpace(applet_interface, origin);
	
	gfx::ObjectHandle skybox = SpawnStarfieldSkybox();
	
	// Create observer.
	auto player_and_camera = SpawnPlayer(sim::Vector3::Zero(), origin);

	// Create vehicle.
	if (spawn_vehicle)
	{
		_vehicle = SpawnRover(sim::Vector3::Zero() + sim::Vector3(0, 5, +5), vehicle_thrust);
	}

	// main loop
	while (applet_interface.Sleep(0))
	{
//		sun1.Call([] (sim::Entity & entity) {
//			auto location = entity.GetLocation();
//			auto time = app::GetTime() * 0.1;
//			auto pos = geom::Vector3d(std::sin(time) * 50000000, std::cos(time) * 50000000, 0);
//			auto & engine = entity.GetEngine();
//			auto & origin = engine.GetOrigin();
//			location->SetTransformation(geom::AbsToRel(pos, origin));
//		});
		
		sun2.Call([] (sim::Entity & entity) {
			auto const & location = entity.GetLocation();
			auto time = app::GetTime() * 0.21314;
			auto pos = geom::Vector3d(std::sin(time) * 70000000, std::cos(time) * 70000000, 0);
			auto const & engine = entity.GetEngine();
			auto const & space = engine.GetSpace();
			location->SetTransformation(space.AbsToRel(pos));
		});

		HandleEvents(applet_interface);
	}
	
	while (! _shapes.empty())
	{
		_shapes.back().Release();
		_shapes.pop_back();
	}
	
	_vehicle.Release();
	sun2.Release();
	sun1.Release();
	moon2.Release();
	moon1.Release();
	planet.Release();
	
	// remove skybox
	skybox.Release();

	player_and_camera[1].Release();
	player_and_camera[0].Release();
}
