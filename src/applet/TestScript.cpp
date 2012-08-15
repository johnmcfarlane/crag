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

#include "AppletInterface_Impl.h"
#include "ObserverScript.h"
#include "Engine.h"

#include "sim/axes.h"
#include "sim/Box.h"
#include "sim/Engine.h"
#include "sim/Firmament.h"
#include "sim/Observer.h"
#include "sim/Planet.h"
#include "sim/Star.h"
#include "sim/Vehicle.h"

#include "form/Engine.h"

#include "gfx/Engine.h"

#include "geom/Transformation.h"

#include "smp/Future.h"

#include "core/Random.h"


DECLARE_CLASS_HANDLE(sim, Ball);		// sim::BallHandle
DECLARE_CLASS_HANDLE(sim, Box);			// sim::BoxHandle
DECLARE_CLASS_HANDLE(sim, Entity);		// sim::EntityHandle
DECLARE_CLASS_HANDLE(sim, Observer);	// sim::ObserverHandle
DECLARE_CLASS_HANDLE(sim, Planet);		// sim::PlanetHandle
DECLARE_CLASS_HANDLE(sim, Star);		// sim::StarHandle
DECLARE_CLASS_HANDLE(sim, Vehicle);		// sim::VehicleHandle


using namespace applet;


namespace
{
	CONFIG_DEFINE (max_distance_from_origin, sim::Scalar, 2500);
	
	////////////////////////////////////////////////////////////////////////////////
	// setup variables
	
	sim::Vector3 observer_start_pos(0, 9999400, -5);
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
	// local types
	
	class TestScript
	{
	public:
		// types
		typedef std::vector<sim::VehicleHandle> EntityVector;
		
		// functions
		void operator() (AppletInterface & applet_interface);
	private:
		void SpawnPlanets();
		void SpawnUniverse();
		void SpawnVehicle();
		void SpawnShapes(int shape_num);
		void HandleEvents();
		void UpdateOrigin();
		
		// variables
		AppletInterface * _applet_interface;
		sim::PlanetHandle _planet, _moon1, _moon2;
		sim::StarHandle _sun;
		sim::FirmamentHandle _skybox;
		sim::ObserverHandle _observer;
		sim::VehicleHandle _vehicle;
		EntityVector _shapes;
		sim::Vector3 _origin = sim::Vector3::Zero();
		core::EventWatcher _event_watcher;
		bool _enable_dynamic_origin = true;
	};

	////////////////////////////////////////////////////////////////////////////////
	// local functions
	
	void add_thruster(sim::VehicleHandle & vehicle_handle, sim::Vector3 const & position, sim::Vector3 const & direction, SDL_Scancode key)
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
}


////////////////////////////////////////////////////////////////////////////////
// TestScript member definitions

// TestScript entry point
void TestScript::operator() (AppletInterface & applet_interface)
{
	DEBUG_MESSAGE("-> Main script");
	
	_applet_interface = & applet_interface;
	
	// Set camera position
	{
		sim::Transformation transformation(observer_start_pos);
		gfx::Daemon::Call([transformation] (gfx::Engine & engine) {
			engine.OnSetCamera(transformation);
		});
	}
	
	// Create planets
	if (spawn_planets)
	{
		SpawnPlanets();
	}
	
	// Give formations time to expand.
	_applet_interface->Sleep(2);

	// Create observer and vehicle.
	{
		_observer.Create(observer_start_pos);
		smp::Handle<ObserverScript> observer_script;
		observer_script.Create(_observer);
	}
	
	SpawnUniverse();
	
	// Create vehicle.
	SpawnVehicle();
	
	// main loop
	while (! _applet_interface->GetQuitFlag())
	{
		HandleEvents();
		
		UpdateOrigin();
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
	DEBUG_MESSAGE("<- Main script");
}

void TestScript::SpawnPlanets()
{
	double planet_radius = 10000000;
	
	_planet.Create(sim::Sphere3(sim::Vector3::Zero(), planet_radius), 3634, 0);
	_moon1.Create(sim::Sphere3(sim::Vector3(planet_radius * 1.5, planet_radius * 2.5, planet_radius * 1.), 1500000), 10, 250);
	_moon2.Create(sim::Sphere3(sim::Vector3(planet_radius * -2.5, planet_radius * 0.5, planet_radius * -1.), 2500000), 13, 0);
}

void TestScript::SpawnUniverse()
{
	// Add the skybox.
	_skybox.Create();
	auto skybox = _skybox;
	gfx::Daemon::Call([skybox] (gfx::Engine & engine) {
		engine.OnSetParent(skybox.GetUid(), gfx::Uid());
	});
	
	// Create sun. 
	_sun.Create(100000000., 30000.);
}

void TestScript::SpawnVehicle()
{
	// Create vehicle
	if (spawn_vehicle)
	{
		sim::Sphere3 sphere;
		sphere.center = observer_start_pos + sim::Vector3(0, 5, +5);
		sphere.radius = 1.;

		_vehicle.Create(sphere);
		
		add_thruster(_vehicle, sim::Vector3(.5, -.8, .5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
		add_thruster(_vehicle, sim::Vector3(.5, -.8, -.5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
		add_thruster(_vehicle, sim::Vector3(-.5, -.8, .5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
		add_thruster(_vehicle, sim::Vector3(-.5, -.8, -.5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
	}
}

void TestScript::SpawnShapes(int shape_num)
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
	sim::Vector3 camera_pos = camera_transformation.GetTranslation();
	sim::Vector3 camera_forward = axes::GetAxis(camera_rotation, axes::FORWARD);
	sim::Vector3 spawn_pos = camera_pos + camera_forward * 5.0;
	
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
				sim::Sphere3 sphere(spawn_pos, std::exp(- GetRandomUnit() * 2));
				ball.Create(sphere);
				_shapes.push_back(ball);
				break;
			}
				
			case 1:
			{
				// box
				sim::Vector3 size(std::exp(GetRandomUnit() * -2.),
							 std::exp(GetRandomUnit() * -2.),
							 std::exp(GetRandomUnit() * -2.));
				
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

void TestScript::HandleEvents()
{
	SDL_Event event;
	while (_event_watcher.PopEvent(event))
	{
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

void TestScript::UpdateOrigin()
{
	if (! _enable_dynamic_origin)
	{
		return;
	}
	
	auto camera_transformation = _applet_interface->Get<gfx::Engine, gfx::Transformation>([] (gfx::Engine & engine) {
		return engine.GetCamera();
	});
	auto camera_pos = camera_transformation.GetTranslation();
	auto origin_to_camera = _origin - camera_pos;
	
	auto distance_from_origin = Length(origin_to_camera);
	if (distance_from_origin > max_distance_from_origin)
	{
		form::Daemon::Call([camera_pos] (form::Engine & engine) {
			engine.SetOrigin(camera_pos);
		});
		_origin = camera_pos;
	}
}

void applet::Test (AppletInterface & applet_interface)
{
	TestScript test_script;
	test_script(applet_interface);
}
