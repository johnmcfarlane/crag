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

#include "Applet.h"
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

#include "physics/Engine.h"

#include "form/Engine.h"
#include "form/node/NodeBuffer.h"

#include "gfx/Engine.h"

#include "core/app.h"

#include "core/ConfigEntry.h"
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
	CONFIG_DEFINE (min_precision_score, sim::Scalar, .001f);
	
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
	// local types
	
	class TestScript
	{
	public:
		// types
		typedef std::vector<sim::VehicleHandle> EntityVector;
		
		// functions
		TestScript()
			: _enable_dynamic_origin(true)
		{
		}

		void operator() (AppletInterface & applet_interface);
	private:
		void SpawnPlanets();
		void SpawnSkybox();
		void SpawnVehicle();
		void SpawnShapes(int shape_num);
		void HandleEvents();
		
		// variables
		AppletInterface * _applet_interface;
		sim::PlanetHandle _planet, _moon1, _moon2;
		sim::StarHandle _sun;
		sim::FirmamentHandle _skybox;
		sim::ObserverHandle _observer;
		sim::VehicleHandle _vehicle;
		EntityVector _shapes;
		core::EventWatcher _event_watcher;
		bool _enable_dynamic_origin;
	};

	////////////////////////////////////////////////////////////////////////////////
	// local functions
	
	void add_thruster(sim::VehicleHandle & vehicle_handle, geom::rel::Vector3 const & position, geom::rel::Vector3 const & direction, SDL_Scancode key)
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

	// Given the camera position relative to the current origin
	// and the distance to the closest bit of geometry,
	// is the origin too far away to allow for precise, camera-centric calculations?
	bool ShouldReviseOrigin(geom::rel::Vector3 const & camera_pos, float min_leaf_distance_squared)
	{
		if (min_leaf_distance_squared == std::numeric_limits<decltype(min_leaf_distance_squared)>::max())
		{
			return true;
		}
		
		ASSERT(min_leaf_distance_squared >= 0);
		auto distance_from_surface = std::sqrt(min_leaf_distance_squared);

		auto distance_from_origin = geom::Length(camera_pos);
		
		auto precision_score = distance_from_surface / distance_from_origin;
		if (precision_score < min_precision_score)
		{
			return true;
		}

		return false;
	}
	
	void ReviseOrigin(sim::Engine & engine)
	{
		auto& physics_engine = engine.GetPhysicsEngine();
		auto& scene = physics_engine.GetScene();
		auto& node_buffer = scene.GetNodeBuffer();

		auto& camera_ray = engine.GetCamera();
		auto& camera_pos = camera_ray.position;
		auto min_leaf_distance_squared = node_buffer.GetMinLeafDistanceSquared();

		if (ShouldReviseOrigin(camera_pos, min_leaf_distance_squared))
		{
#if ! defined(NDEBUG)
			app::Beep();
#endif

			auto origin = engine.GetOrigin();
			auto new_origin = geom::RelToAbs(camera_pos, origin);

			DEBUG_MESSAGE("Set: %f,%f,%f", new_origin.x, new_origin.y, new_origin.z);
			engine.SetOrigin(new_origin);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// TestScript member definitions

// TestScript entry point
void TestScript::operator() (AppletInterface & applet_interface)
{
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
		smp::Handle<ObserverScript> observer_script;
		observer_script.Create(_observer);
	}
	
	// Create origin controller.
	{
		Daemon::Call([] (Engine & engine) {
			auto functor = [] (AppletInterface & applet_interface)
			{
				while (! applet_interface.GetQuitFlag())
				{
					applet_interface.Sleep(0.23432);
					sim::Daemon::Call(& ReviseOrigin);
				}
			};
			
			engine.Launch(functor, 8192, "Main");
		});
	}
	
	
	SpawnSkybox();
	
	// Create vehicle.
	SpawnVehicle();
	
	// main loop
	while (! _applet_interface->GetQuitFlag())
	{
		applet_interface.WaitFor([this, & applet_interface] () {
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

void TestScript::SpawnPlanets()
{
	sim::Scalar planet_radius = 10000000;
	
	_planet.Create(sim::Sphere3(sim::Vector3::Zero(), planet_radius), 3634, 0);
	_moon1.Create(sim::Sphere3(sim::Vector3(planet_radius * 1.5f, planet_radius * 2.5f, planet_radius * 1.f), 1500000), 10, 250);
	_moon2.Create(sim::Sphere3(sim::Vector3(planet_radius * -2.5f, planet_radius * 0.5f, planet_radius * -1.f), 2500000), 13, 0);
}

void TestScript::SpawnSkybox()
{
	// Add the skybox.
	_skybox.Create();
	auto skybox = _skybox;
	gfx::Daemon::Call([skybox] (gfx::Engine & engine) {
		engine.OnSetParent(skybox.GetUid(), gfx::Uid());
	});
}

void TestScript::SpawnVehicle()
{
	// Create vehicle
	if (spawn_vehicle)
	{
		geom::rel::Sphere3 sphere;
		sphere.center = geom::Cast<float>(observer_start_pos + geom::abs::Vector3(0, 5, +5));
		sphere.radius = 1.;

		_vehicle.Create(sphere);
		
		add_thruster(_vehicle, geom::rel::Vector3(.5, -.8f, .5), geom::rel::Vector3(0, 5, 0), SDL_SCANCODE_H);
		add_thruster(_vehicle, geom::rel::Vector3(.5, -.8f, -.5), geom::rel::Vector3(0, 5, 0), SDL_SCANCODE_H);
		add_thruster(_vehicle, geom::rel::Vector3(-.5, -.8f, .5), geom::rel::Vector3(0, 5, 0), SDL_SCANCODE_H);
		add_thruster(_vehicle, geom::rel::Vector3(-.5, -.8f, -.5), geom::rel::Vector3(0, 5, 0), SDL_SCANCODE_H);
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

void TestScript::HandleEvents()
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

void applet::Test (AppletInterface & applet_interface)
{
	TestScript test_script;
	test_script(applet_interface);
}
