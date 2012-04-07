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

#include "sim/Box.h"
#include "sim/Engine.h"
#include "sim/Planet.h"
#include "sim/Star.h"
#include "sim/Vehicle.h"

#include "gfx/Engine.h"

#include "geom/Transformation.h"

#include "core/Random.h"


DECLARE_CLASS_HANDLE(sim, Ball);	// sim::BallHandle
DECLARE_CLASS_HANDLE(sim, Box);		// sim::BoxHandle
DECLARE_CLASS_HANDLE(sim, Planet);	// sim::PlanetHandle
DECLARE_CLASS_HANDLE(sim, Star);	// sim::StarHandle
DECLARE_CLASS_HANDLE(sim, Vehicle);	// sim::VehicleHandle


using namespace script;


namespace
{
	////////////////////////////////////////////////////////////////////////////////
	// setup variables
	
	sim::Vector3 observer_start_pos(0, 9999400, 0);
	size_t max_shapes = 50;
	Time shape_drop_period = .5;
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
	int GetRandomInt(int n)
	{
		return random_sequence.GetInt(n);
	}
	
	////////////////////////////////////////////////////////////////////////////////
	// local types
	
	class TestScript
	{
	public:
		// types
		typedef std::vector<sim::VehicleHandle> EntityVector;
		
		// functions
		void operator() (FiberInterface & fiber);
	private:
		void SpawnUniverse();
		void SpawnVehicle();
		void SpawnShapes();
		
		// variables
		sim::PlanetHandle _planet, _moon1, _moon2;
		sim::StarHandle _sun;
		sim::VehicleHandle _vehicle;
		EntityVector _shapes;
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

void TestScript::operator() (FiberInterface & fiber)
{
	// TODO: Take a good look at the callstack that gets you here
	// TODO: and have long think about what you've done.
	DEBUG_MESSAGE("-> Main script");
	
	// Set camera position
	{
		sim::Transformation transformation(observer_start_pos);
		gfx::Daemon::Call(& gfx::Engine::OnSetCamera, transformation);
	}
	
	SpawnUniverse();

	// Give formations time to expand.
	fiber.Sleep(2);

	// Create observer and vehicle.
	fiber.Launch(ref(new ObserverScript(observer_start_pos)));
	
	SpawnVehicle();
	
	// main loop
	while (! fiber.GetQuitFlag())
	{
		fiber.Sleep(shape_drop_period);
		
		SpawnShapes();
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
	DEBUG_MESSAGE("<- Main script");
}

void TestScript::SpawnUniverse()
{
	// Create planets
	if (spawn_planets)
	{
		double planet_radius = 10000000;
		sim::Planet::InitData init_data;
		
		init_data.sphere.center = Vector3d::Zero();
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
		sphere.center = observer_start_pos + sim::Vector3(0, 5, 0);
		sphere.radius = 1.;

		_vehicle.Create(sphere);
		
		add_thruster(_vehicle, sim::Vector3(.5, -.8, .5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
		add_thruster(_vehicle, sim::Vector3(.5, -.8, -.5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
		add_thruster(_vehicle, sim::Vector3(-.5, -.8, .5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
		add_thruster(_vehicle, sim::Vector3(-.5, -.8, -.5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
	}
}

void TestScript::SpawnShapes()
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
			first->Destroy();
			_shapes.erase(first);
		}
	}
	
	if (_shapes.size() < max_shapes)
	{
		Vector3d spawn_pos(GetRandomUnitInclusive() - .5,
						   observer_start_pos.y,
						   -4.5 + GetRandomUnit());
		
		switch (GetRandomInt(2))
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
					Vector3d(std::exp(GetRandomUnit() * -2.),
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


void script::Test (FiberInterface & fiber)
{
	TestScript test_script;
	test_script(fiber);
}
