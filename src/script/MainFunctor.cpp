//
//  main.cpp
//  crag
//
//  Created by John McFarlane on 2012-02-09.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "MainFunctor.h"

#include "ObserverFunctor.h"

#include "sim/Box.h"
#include "sim/Planet.h"
#include "sim/Star.h"
#include "sim/Vehicle.h"

#include "gfx/Renderer.h"

#include "geom/Transformation.h"

#include "core/Random.h"


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
	
	class AddThrusterFunctor
	{
	public:
		
		// functions
		AddThrusterFunctor(sim::Vehicle::Thruster const & thruster)
		: _thruster(thruster)
		{
			Assert(_thruster.gfx_uid == gfx::Uid::null);
		}
		
		void operator() (sim::Vehicle * vehicle) const
		{
			if (vehicle == nullptr)
			{
				Assert(false);
				return;
			}
			
			vehicle->AddThruster(_thruster);
		}
		
	private:
		sim::Vehicle::Thruster _thruster;
	};
	
	////////////////////////////////////////////////////////////////////////////////
	// local functions
	
	void add_thruster(EntityHandle<sim::Vehicle> & vehicle, sim::Vector3 const & position, sim::Vector3 const & direction, SDL_Scancode key)
	{
		sim::Vehicle::Thruster thruster;
		thruster.position = position;
		thruster.direction = direction;
		thruster.key = SDL_SCANCODE_H;
		thruster.thrust_factor = 1.;
		
		AddThrusterFunctor functor(thruster);
		vehicle.Call<AddThrusterFunctor>(functor);
	}
}

void MainFunctor::operator() (FiberInterface & fiber)
{
	std::cout << "-> Main script" << std::endl;
	
	// Set camera position
	{
		sim::Transformation transformation(observer_start_pos);
		gfx::Daemon::Call(transformation, & gfx::Renderer::OnSetCamera);
	}
	
	SpawnUniverse();

	// Give formations time to expand.
	fiber.Sleep(2);

	// Create observer and vehicle.
	fiber.Launch(ref(new ObserverFunctor(observer_start_pos)));
	
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
	std::cout << "<- Main script" << std::endl;
}

void MainFunctor::SpawnUniverse()
{
	// Create planets
	{
		double planet_radius = 10000000;
		sim::InitData<sim::Planet> init_data;
		
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
		sim::InitData<sim::Star> init_data =
		{
			100000000.,	// sun_orbit_distance
			30000.	// sun_year
		};

		_sun.Create(init_data);
	}
}

void MainFunctor::SpawnVehicle()
{
	// Create vehicle
	if (spawn_vehicle)
	{
		sim::InitData<sim::Vehicle> init_data;
		init_data.sphere.center = observer_start_pos + sim::Vector3(0, 5, 0);
		init_data.sphere.radius = 1.;

		_vehicle.Create(init_data);
	}
	
	add_thruster(_vehicle, sim::Vector3(.5, -.8, .5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
	add_thruster(_vehicle, sim::Vector3(.5, -.8, -.5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
	add_thruster(_vehicle, sim::Vector3(-.5, -.8, .5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
	add_thruster(_vehicle, sim::Vector3(-.5, -.8, -.5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
}

void MainFunctor::SpawnShapes()
{
	if (max_shapes == 0)
	{
		return;
	}
	
	if (cleanup_shapes)
	{
		if (_shapes.size() > max_shapes)
		{
			_shapes.erase(_shapes.begin());
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
				sim::InitData<sim::Ball> init_data = 
				{
					sim::Sphere3(spawn_pos, std::exp(- GetRandomUnit() * 2))
				};
				
				EntityHandle<sim::Ball> ball;
				ball.Create(init_data);
				_shapes.push_back(ball);
				break;
			}
				
			case 1:
			{
				// box
				sim::InitData<sim::Box> init_data = 
				{
					spawn_pos, 
					Vector3d(std::exp(GetRandomUnit() * -2.),
							 std::exp(GetRandomUnit() * -2.),
							 std::exp(GetRandomUnit() * -2.))
				};
				
				EntityHandle<sim::Box> box;
				box.Create(init_data);
				_shapes.push_back(box);
				break;
			}
				
			default:
				Assert(false);
		}
	}
}
