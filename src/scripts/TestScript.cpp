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
#include "TestScript.h"
#include "MonitorOrigin.h"

#include "applet/Applet.h"
#include "applet/AppletInterface_Impl.h"
#include "applet/Engine.h"

#include "sim/axes.h"
#include "sim/Engine.h"
#include "sim/Entity.h"
#include "sim/Firmament.h"

#include "physics/Location.h"

#include "form/Engine.h"
#include "form/node/NodeBuffer.h"

#include "gfx/Engine.h"

#include "core/app.h"

#include "core/app.h"
#include "core/EventWatcher.h"
#include "core/Random.h"

using geom::Vector3f;

namespace sim 
{ 
	DECLARE_CLASS_HANDLE(Entity);// sim::EntityHandle
}

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

	applet::AppletInterface * _applet_interface;
	sim::EntityHandle _planet, _moon1, _moon2;
	sim::EntityHandle _sun;
	sim::FirmamentHandle _skybox;	// TODO: This can be cast to an entity handle!!
	sim::EntityHandle _vehicle;
	EntityVector _shapes;
	core::EventWatcher _event_watcher;
	bool _enable_dynamic_origin = true;
	
	////////////////////////////////////////////////////////////////////////////////
	// functions
	
	void SpawnShapes(sim::EntityHandle observer, int shape_num)
	{
		if (max_shapes == 0)
		{
			return;
		}
	
		smp::Future<sim::Transformation> camera_transformation_future = _applet_interface->Get<sim::Engine, sim::Transformation>(observer, [] (sim::Entity & observer) -> sim::Transformation {
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
				sim::EntityHandle ball = SpawnBall(spawn_pos, color);
				_shapes.push_back(ball);
				break;
			}
			
			case 1:
			{
				// box
				sim::EntityHandle box = SpawnBox(spawn_pos, color);
				_shapes.push_back(box);
				break;
			}
			
			default:
				ASSERT(false);
		}
	}

	void HandleEvents(sim::EntityHandle observer)
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
	}
}

// main entry point
void Test (applet::AppletInterface & applet_interface)
{
	FUNCTION_NO_REENTRY;

	_applet_interface = & applet_interface;
	
	// Set camera position
	{
		sim::Transformation transformation(geom::Cast<sim::Scalar>(observer_start_pos));
		gfx::Daemon::Call([transformation] (gfx::Engine & engine) {
			engine.OnSetCamera(transformation);
		});
	}
	
	// Create sun. 
	_sun = SpawnStar();
	
	// Create planets
	if (spawn_planets)
	{
		sim::Scalar planet_radius = 10000000;

		_planet = SpawnPlanet(sim::Sphere3(sim::Vector3::Zero(), planet_radius), 3634, 0);
		_moon1 = SpawnPlanet(sim::Sphere3(sim::Vector3(planet_radius * 1.5f, planet_radius * 2.5f, planet_radius * 1.f), 1500000), 10, 250);
		_moon2 = SpawnPlanet(sim::Sphere3(sim::Vector3(planet_radius * -2.5f, planet_radius * 0.5f, planet_radius * -1.f), 2500000), 13, 0);
	}
	
	// Give formations time to expand.
	_applet_interface->Sleep(2);

	// Create observer.
	sim::EntityHandle observer = SpawnObserver(observer_start_pos);

	// Create origin controller.
	applet_interface.Launch("MonitorOrigin", 8192, &MonitorOrigin);
	
	_skybox = SpawnSkybox();
	
	// Create vehicle.
	if (spawn_vehicle)
	{
		_vehicle = SpawnVehicle(observer_start_pos + geom::rel::Vector3(0, 5, +5));
	}

	// main loop
	while (! _applet_interface->GetQuitFlag())
	{
		applet_interface.WaitFor([& applet_interface] () {
			return ! _event_watcher.IsEmpty() || applet_interface.GetQuitFlag();
		});

		HandleEvents(observer);
	}
	
	while (! _shapes.empty())
	{
		_shapes.back().Destroy();
		_shapes.pop_back();
	}
	
	_vehicle.Destroy();
	_sun.Destroy();
	_moon2.Destroy();
	_moon1.Destroy();
	_planet.Destroy();
	
	// remove skybox
	_skybox.Destroy();

	observer.Destroy();
	
	ASSERT(_applet_interface == & applet_interface);
}
