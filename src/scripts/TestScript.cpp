//
//  TestScript.cpp
//  crag
//
//  Created by John McFarlane on 2012-02-09.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "TestScript.h"

#include "ObserverScript.h"
#include "MonitorOrigin.h"

#include "applet/Applet.h"
#include "applet/AppletInterface_Impl.h"
#include "applet/Engine.h"

#include "sim/axes.h"
#include "sim/Box.h"
#include "sim/Engine.h"
#include "sim/Firmament.h"
#include "sim/Observer.h"
#include "sim/Planet.h"
#include "sim/Star.h"
#include "sim/Vehicle.h"

#include "physics/Engine.h"

#include "form/Engine.h"
#include "form/node/NodeBuffer.h"

#include "gfx/Engine.h"

#include "core/app.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"

namespace sim 
{ 
	DECLARE_CLASS_HANDLE(Ball);	// sim::BallHandle
	DECLARE_CLASS_HANDLE(Box);// sim::BoxHandle
	DECLARE_CLASS_HANDLE(Entity);// sim::EntityHandle
	DECLARE_CLASS_HANDLE(Observer);	// sim::ObserverHandle
	DECLARE_CLASS_HANDLE(Planet);// sim::PlanetHandle
	DECLARE_CLASS_HANDLE(Star);// sim::StarHandle
	DECLARE_CLASS_HANDLE(Vehicle);// sim::VehicleHandle
}

namespace
{
	////////////////////////////////////////////////////////////////////////////////
	// types
	typedef std::vector<sim::VehicleHandle> EntityVector;
		
	////////////////////////////////////////////////////////////////////////////////
	// setup variables
	
	geom::abs::Vector3 observer_start_pos(0, 9999400, -5);
	size_t max_shapes = 50;
	bool cleanup_shapes = true;
	bool spawn_vehicle = true;
	bool spawn_planets = true;
	
	////////////////////////////////////////////////////////////////////////////////
	// random number generation
	
	Random random_sequence;
	double GetRandomUnit()
	{
		return random_sequence.GetUnit<double>();
	}
	
	////////////////////////////////////////////////////////////////////////////////
	// variables

	applet::AppletInterface * _applet_interface;
	sim::PlanetHandle _planet, _moon1, _moon2;
	sim::StarHandle _sun;
	sim::FirmamentHandle _skybox;
	sim::ObserverHandle _observer;
	sim::VehicleHandle _vehicle;
	EntityVector _shapes;
	core::EventWatcher _event_watcher;
	bool _enable_dynamic_origin = true;
	
	////////////////////////////////////////////////////////////////////////////////
	// functions
	
	void SpawnShapes(int shape_num)
	{
		if (max_shapes == 0)
		{
			return;
		}
	
		smp::Future<sim::Transformation> camera_transformation_future = _applet_interface->Get<sim::Engine, sim::Transformation>(_observer, [] (sim::Observer & observer) -> sim::Transformation {
			return observer.GetTransformation();
		});
	
		sim::Transformation camera_transformation = camera_transformation_future.Get();
		sim::Matrix33 camera_rotation = camera_transformation.GetRotation();
		geom::rel::Vector3 camera_pos = camera_transformation.GetTranslation();
		geom::rel::Vector3 camera_forward = axes::GetAxis(camera_rotation, axes::FORWARD);
		geom::rel::Vector3 spawn_pos = camera_pos + camera_forward * geom::rel::Scalar(5);
	
		if (cleanup_shapes)
		{
			if (_shapes.size() >= max_shapes)
			{
				EntityVector::iterator first = _shapes.begin();
				first->Destroy();
				_shapes.erase(first);
			}
		}
	
		if (_shapes.size() < max_shapes)
		{
			switch (shape_num)
			{
				case 0:
				{
					// ball
					sim::BallHandle ball;
					geom::rel::Sphere3 sphere(spawn_pos, geom::rel::Scalar(std::exp(- GetRandomUnit() * 2)));
					ball.Create(sphere);
					_shapes.push_back(ball);
					break;
				}
				
				case 1:
				{
					// box
					geom::rel::Vector3 size(geom::rel::Scalar(std::exp(GetRandomUnit() * -2.)),
								 geom::rel::Scalar(std::exp(GetRandomUnit() * -2.)),
								 geom::rel::Scalar(std::exp(GetRandomUnit() * -2.)));
				
					sim::BoxHandle box;
					box.Create(spawn_pos, size);
					_shapes.push_back(box);
					break;
				}
				
				default:
					ASSERT(false);
			}
		}
	}

	void HandleEvents()
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
					SpawnShapes(0);
					break;
				
				case SDL_SCANCODE_PERIOD:
					SpawnShapes(1);
					break;
				
				default:
					break;
			}
		}
	}

	void SpawnPlanets()
	{
		sim::Scalar planet_radius = 10000000;
	
		_planet.Create(sim::Sphere3(sim::Vector3::Zero(), planet_radius), 3634, 0);
		_moon1.Create(sim::Sphere3(sim::Vector3(planet_radius * 1.5f, planet_radius * 2.5f, planet_radius * 1.f), 1500000), 10, 250);
		_moon2.Create(sim::Sphere3(sim::Vector3(planet_radius * -2.5f, planet_radius * 0.5f, planet_radius * -1.f), 2500000), 13, 0);
	}

	void SpawnSkybox()
	{
		// Add the skybox.
		_skybox.Create();
		auto skybox = _skybox;
		gfx::Daemon::Call([skybox] (gfx::Engine & engine) {
			engine.OnSetParent(skybox.GetUid(), gfx::Uid());
		});
	}

	void AddThruster(sim::VehicleHandle & vehicle_handle, geom::rel::Vector3 const & position, geom::rel::Vector3 const & direction, SDL_Scancode key)
	{
		sim::Vehicle::Thruster thruster;
		thruster.position = position;
		thruster.direction = direction;
		thruster.key = SDL_SCANCODE_H;
		thruster.thrust_factor = 1.;
		
		vehicle_handle.Call([thruster] (sim::Vehicle & vehicle) {
			vehicle.AddThruster(thruster);
		});
	}

	void SpawnVehicle()
	{
		// Create vehicle
		if (spawn_vehicle)
		{
			geom::rel::Sphere3 sphere;
			sphere.center = geom::Cast<float>(observer_start_pos + geom::abs::Vector3(0, 5, +5));
			sphere.radius = 1.;

			_vehicle.Create(sphere);
		
			AddThruster(_vehicle, geom::rel::Vector3(.5, -.8f, .5), geom::rel::Vector3(0, 5, 0), SDL_SCANCODE_H);
			AddThruster(_vehicle, geom::rel::Vector3(.5, -.8f, -.5), geom::rel::Vector3(0, 5, 0), SDL_SCANCODE_H);
			AddThruster(_vehicle, geom::rel::Vector3(-.5, -.8f, .5), geom::rel::Vector3(0, 5, 0), SDL_SCANCODE_H);
			AddThruster(_vehicle, geom::rel::Vector3(-.5, -.8f, -.5), geom::rel::Vector3(0, 5, 0), SDL_SCANCODE_H);
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
	_sun.Create(100000000., 30000.);
	
	// Create planets
	if (spawn_planets)
	{
		SpawnPlanets();
	}
	
	// Give formations time to expand.
	_applet_interface->Sleep(2);

	// Create observer.
	{
		_observer.Create(observer_start_pos);
		smp::Handle<applet::ObserverScript> observer_script;
		observer_script.Create(_observer);
	}
	
	// Create origin controller.
	{
		applet::Daemon::Call([] (applet::Engine & engine) {
			engine.Launch(&MonitorOrigin, 8192, "MonitorOrigin");
		});
	}
	
	
	SpawnSkybox();
	
	// Create vehicle.
	SpawnVehicle();
	
	// main loop
	while (! _applet_interface->GetQuitFlag())
	{
		applet_interface.WaitFor([& applet_interface] () {
			return ! _event_watcher.IsEmpty() || applet_interface.GetQuitFlag();
		});

		HandleEvents();
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
	
	ASSERT(_applet_interface == & applet_interface);
}
