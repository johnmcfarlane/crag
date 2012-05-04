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
#include "EventCondition.h"
#include "ObserverScript.h"
#include "Engine.h"

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

#include "core/Random.h"

#include <fstream>


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
	double GetRandomUnitInclusive()
	{
		return random_sequence.GetUnitInclusive<double>();
	}
	
	////////////////////////////////////////////////////////////////////////////////
	// local types
	
	// EventCondition which blocks until Ctrl-I is pressed.
	// But you can just call PopEvent to get any existing Ctrl-I events.
	class KeyPressEventCondition : public EventCondition
	{
		virtual bool Filter(SDL_Event const & event) const final
		{
			return event.type == SDL_KEYDOWN;
		}
	};
	
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
		KeyPressEventCondition _key_press_events;
		bool _enable_dynamic_origin = true;
	};

	////////////////////////////////////////////////////////////////////////////////
	// local functions
	
	void add_thruster(sim::VehicleHandle & vehicle, sim::Vector3 const & position, sim::Vector3 const & direction, SDL_Scancode key)
	{
		sim::Vehicle::Thruster thruster;
		thruster.position = position;
		thruster.direction = direction;
		thruster.key = SDL_SCANCODE_H;
		thruster.thrust_factor = 1.;
		
		vehicle.Call(& sim::Vehicle::AddThruster, thruster);
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
		gfx::Daemon::Call(& gfx::Engine::OnSetCamera, transformation);
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
		AppletBase * observer_script = new ObserverScript(_observer);
		Daemon::Call(& Engine::OnAddObject, observer_script);
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
	sim::Planet::InitData init_data;
	
	init_data.sphere.center = geom::Vector3d::Zero();
	init_data.sphere.radius = planet_radius;
	init_data.random_seed = 3634;
	init_data.num_craters = 0;
	_planet.Create(init_data);
	
	init_data.sphere.center.x = planet_radius * 1.5;
	init_data.sphere.center.y = planet_radius * 2.5;
	init_data.sphere.center.z = planet_radius * 1.;
	init_data.sphere.radius = 1500000;
	init_data.random_seed = 10;
	init_data.num_craters = 250;
	_moon1.Create(init_data);
	
	init_data.sphere.center.x = planet_radius * -2.5;
	init_data.sphere.center.y = planet_radius * 0.5;
	init_data.sphere.center.z = planet_radius * -1.;
	init_data.sphere.radius = 2500000;
	init_data.random_seed = 13;
	init_data.num_craters = 0;
	_moon2.Create(init_data);
}

void TestScript::SpawnUniverse()
{
	// Add the skybox.
	_skybox.Create();
	gfx::Daemon::Call(& gfx::Engine::OnSetParent, _skybox.GetUid(), gfx::Uid());
	
	// Create sun. 
	{
		sim::Star::InitData init_data =
		{
			100000000.,	// sun_orbit_distance
			30000.	// sun_year
		};

		_sun.Create(init_data);
	}
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
	
	Future<sim::Transformation> camera_pos(* _applet_interface, static_cast<sim::EntityHandle &>(_observer), & sim::Observer::GetTransformation);
	
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
		geom::Vector3d spawn_pos(GetRandomUnitInclusive() - .5,
						   observer_start_pos.y,
						   -4.5 + GetRandomUnit());
		
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
				sim::Box::InitData init_data = 
				{
					spawn_pos, 
					geom::Vector3d(std::exp(GetRandomUnit() * -2.),
							 std::exp(GetRandomUnit() * -2.),
							 std::exp(GetRandomUnit() * -2.))
				};
				
				sim::BoxHandle box;
				box.Create(init_data);
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
	while (_key_press_events.PopEvent(event))
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
	
	auto camera_transformation = _applet_interface->Call(& gfx::Engine::GetCamera);
	auto camera_pos = camera_transformation.GetTranslation();
	auto origin_to_camera = _origin - camera_pos;
	
	auto distance_from_origin = Length(origin_to_camera);
	if (distance_from_origin > max_distance_from_origin)
	{
		_applet_interface->Call<form::Engine, sim::Vector3>(& form::Engine::SetOrigin, camera_pos);
		_origin = camera_pos;
	}
}

void applet::Test (AppletInterface & applet_interface)
{
	TestScript test_script;
	test_script(applet_interface);
}
