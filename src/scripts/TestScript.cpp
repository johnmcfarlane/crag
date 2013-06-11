//
//  TestScript.cpp
//  crag
//
//  Created by John McFarlane on 2012-02-09.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "SpawnEntityFunctions.h"
#include "MonitorOrigin.h"
#include "RegulatorScript.h"

#include "applet/Applet.h"
#include "applet/AppletInterface_Impl.h"

#include "sim/axes.h"
#include "sim/Engine.h"
#include "sim/Entity.h"

#include "physics/Location.h"

#include "gfx/Color.h"
#include "gfx/Engine.h"
#include "gfx/object/Object.h"
#include "gfx/SetCameraEvent.h"

#include "geom/origin.h"

#include "core/EventWatcher.h"
#include "core/Random.h"

using geom::Vector3f;

namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// types
	typedef std::vector<sim::EntityHandle> EntityVector;
		
	////////////////////////////////////////////////////////////////////////////////
	// setup variables
	
	geom::rel::Vector3 observer_start_pos(0, 9999400, -5);
	size_t max_shapes = 50;
	bool cleanup_shapes = true;
	bool spawn_vehicle = true;
	bool spawn_planets = true;
	
	////////////////////////////////////////////////////////////////////////////////
	// variables

	Random random_sequence;
	applet::AppletInterface * _applet_interface;
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
	
	void SpawnShapes(sim::EntityHandle observer, int shape_num)
	{
		if (max_shapes == 0)
		{
			return;
		}
	
		ipc::Future<sim::Transformation> camera_transformation_future = _applet_interface->Get<sim::Engine, sim::Transformation>(observer, [] (sim::Entity & observer) -> sim::Transformation {
			auto location = observer.GetLocation();
			return location->GetTransformation();
		});
	
		if (cleanup_shapes)
		{
			if (_shapes.size() >= max_shapes)
			{
				EntityVector::iterator first = _shapes.begin();
				first->Destroy();
				_shapes.erase(first);
			}
		}
	
		if (_shapes.size() >= max_shapes)
		{
			return;
		}

		sim::Transformation camera_transformation = camera_transformation_future.Get();
		sim::Matrix33 camera_rotation = camera_transformation.GetRotation();
		geom::rel::Vector3 camera_pos = camera_transformation.GetTranslation();
		geom::rel::Vector3 camera_forward = axes::GetAxis(camera_rotation, axes::FORWARD);
		geom::rel::Vector3 spawn_pos = camera_pos + camera_forward * geom::rel::Scalar(5);
	
		gfx::Color4f color(Random::sequence.GetUnitInclusive<float>(), 
					Random::sequence.GetUnitInclusive<float>(), 
					Random::sequence.GetUnitInclusive<float>(), 
					Random::sequence.GetUnitInclusive<float>());

		switch (shape_num)
		{
			case 0:
			{
				// ball
				float radius = geom::rel::Scalar(std::exp(- GetRandomUnit() * 2));
				sim::EntityHandle ball = SpawnBall(spawn_pos, radius, color);
				_shapes.push_back(ball);
				break;
			}
			
			case 1:
			{
				// box
				geom::rel::Vector3 size(geom::rel::Scalar(std::exp(GetRandomUnit() * -2.)),
					geom::rel::Scalar(std::exp(GetRandomUnit() * -2.)),
					geom::rel::Scalar(std::exp(GetRandomUnit() * -2.)));
				sim::EntityHandle box = SpawnBox(spawn_pos, size, color);
				_shapes.push_back(box);
				break;
			}
			
			default:
				ASSERT(false);
		}
	}

	// returns true if the applet should NOT quit
	bool HandleEvents(sim::EntityHandle observer)
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
					return false;
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
					SpawnShapes(observer, 0);
					break;
				
				case SDL_SCANCODE_PERIOD:
					SpawnShapes(observer, 1);
					break;
				
				default:
					break;
			}
		}

		return true;
	}
}

// main entry point
void Test (applet::AppletInterface & applet_interface)
{
	FUNCTION_NO_REENTRY;

	_applet_interface = & applet_interface;
	
	// Set camera position
	{
		gfx::SetCameraEvent event;
		event.transformation = geom::Cast<sim::Scalar>(observer_start_pos);
		gfx::Daemon::Broadcast(event);
	}
	
	// Create sun. 
	sim::EntityHandle sun = SpawnStar();
	
	// Create planets
	sim::EntityHandle planet, moon1, moon2;
	if (spawn_planets)
	{
		sim::Scalar planet_radius = 10000000;

		planet = SpawnPlanet(sim::Sphere3(sim::Vector3::Zero(), planet_radius), 3634, 0);
		moon1 = SpawnPlanet(sim::Sphere3(sim::Vector3(planet_radius * 1.5f, planet_radius * 2.5f, planet_radius * 1.f), 1500000), 10, 250);
		moon2 = SpawnPlanet(sim::Sphere3(sim::Vector3(planet_radius * -2.5f, planet_radius * 0.5f, planet_radius * -1.f), 2500000), 13, 0);
	}
	
	// Give formations time to expand.
	_applet_interface->Sleep(2);

	// Create observer.
	sim::EntityHandle observer = SpawnObserver(observer_start_pos);

	// Create origin controller.
	applet_interface.Launch("MonitorOrigin", 8192, &MonitorOrigin);
	
	// launch regulator
	applet_interface.Launch("Regulator", 8192, [] (applet::AppletInterface & applet_interface) {
		script::RegulatorScript regulator;
		regulator(applet_interface);
	});
	
	gfx::ObjectHandle skybox = SpawnSkybox();
	
	// Create vehicle.
	if (spawn_vehicle)
	{
		_vehicle = SpawnRover(observer_start_pos + geom::rel::Vector3(0, 5, +5));
	}

	// main loop
	while (! _applet_interface->GetQuitFlag())
	{
		applet_interface.WaitFor([& applet_interface] () {
			return ! _event_watcher.IsEmpty() || applet_interface.GetQuitFlag();
		});

		if (! HandleEvents(observer))
		{
			break;
		}
	}
	
	while (! _shapes.empty())
	{
		_shapes.back().Destroy();
		_shapes.pop_back();
	}
	
	_vehicle.Destroy();
	sun.Destroy();
	moon2.Destroy();
	moon1.Destroy();
	planet.Destroy();
	
	// remove skybox
	skybox.Destroy();

	observer.Destroy();
	
	ASSERT(_applet_interface == & applet_interface);
}
