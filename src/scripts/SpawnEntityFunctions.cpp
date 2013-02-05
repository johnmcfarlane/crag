//
//  SpawnEntityFunctions.cpp
//  crag
//
//  Created by John McFarlane on 2013-02-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "SpawnEntityFunctions.h"

#include "planet/sim/PlanetController.h"
#include "planet/physics/PlanetBody.h"
#include "planet/gfx/Planet.h"

#include "sim/Engine.h"
#include "sim/Entity.h"
#include "sim/EntityFunctions.h"
#include "sim/Firmament.h"
#include "sim/ObserverController.h"
#include "sim/VehicleController.h"

#include "physics/BoxBody.h"
#include "physics/Engine.h"
#include "physics/FixedLocation.h"
#include "physics/SphericalBody.h"

#include "gfx/Engine.h"
#include "gfx/object/Ball.h"
#include "gfx/object/Box.h"
#include "gfx/object/Light.h"

#include "geom/origin.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"

namespace sim 
{ 
	DECLARE_CLASS_HANDLE(Firmament); //FirmamentHandle
}

namespace
{
	////////////////////////////////////////////////////////////////////////////////
	// Config values
	
	CONFIG_DEFINE (box_density, physics::Scalar, 1);
	CONFIG_DEFINE (box_linear_damping, physics::Scalar, 0.005f);
	CONFIG_DEFINE (box_angular_damping, physics::Scalar, 0.005f);

	CONFIG_DEFINE (ball_density, float, 1);
	CONFIG_DEFINE (ball_linear_damping, float, 0.005f);
	CONFIG_DEFINE (ball_angular_damping, float, 0.005f);

	CONFIG_DEFINE (observer_radius, float, .5);
	CONFIG_DEFINE (observer_density, float, 1);
	
	CONFIG_DEFINE (observer_linear_damping, physics::Scalar, 0.025f);
	CONFIG_DEFINE (observer_angular_damping, physics::Scalar, 0.05f);

	CONFIG_DEFINE (observer_light_color, geom::Vector3f, geom::Vector3f(0.6f, 0.8f, 1.0f) * 1.f);

	////////////////////////////////////////////////////////////////////////////////
	// function definitions
	
	// random number generation
	Random random_sequence;
	double GetRandomUnit()
	{
		return random_sequence.GetUnit<double>();
	}
	
	void ConstructBox(sim::Entity & box, geom::rel::Vector3 spawn_pos, geom::rel::Vector3 size, gfx::Color4f color)
	{
		// physics
		sim::Engine & engine = box.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto & body = * new physics::BoxBody(physics_engine, true, size);
		body.SetPosition(spawn_pos);
		body.SetDensity(box_density);
		body.SetLinearDamping(box_linear_damping);
		body.SetAngularDamping(box_angular_damping);
		box.SetLocation(& body);

		// graphics
		auto model = sim::AddModelWithTransform<gfx::Box>(color);

		box.SetModel(model);
	}

	void ConstructSphericalBody(sim::Entity & entity, geom::rel::Sphere3 const & sphere, float density, float linear_damping, float angular_damping)
	{
		sim::Engine & engine = entity.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto & body = * new physics::SphericalBody(physics_engine, true, sphere.radius);
		body.SetPosition(sphere.center);
		body.SetDensity(density);
		body.SetLinearDamping(linear_damping);
		body.SetAngularDamping(angular_damping);
		entity.SetLocation(& body);
	}

	void ConstructBall(sim::Entity & ball, geom::rel::Sphere3 sphere, gfx::Color4f color)
	{
		// physics
		ConstructSphericalBody(ball, sphere, ball_density, ball_linear_damping, ball_angular_damping);

		// graphics
		gfx::BranchNodeHandle model = sim::AddModelWithTransform<gfx::Ball>(color);
		ball.SetModel(model);
	}

	void ConstructObserver(sim::Entity & observer, sim::Vector3 const & position)
	{
		// physics
		ConstructSphericalBody(observer, geom::rel::Sphere3(position, observer_radius), observer_density, observer_linear_damping, observer_angular_damping);

		// controller
		auto controller = new sim::ObserverController(observer);
		observer.SetController(controller);

#if defined(OBSERVER_LIGHT)
		// register light with the renderer
		gfx::Light * light = new gfx::Light(observer_light_color);
		_light_uid = AddModelWithTransform(* light);
#endif
	}

	void AddThruster(sim::VehicleController & vehicle_controller, sim::Vector3 const & position, sim::Vector3 const & direction, SDL_Scancode key)
	{
		sim::VehicleController::Thruster thruster;
		thruster.position = position;
		thruster.direction = direction;
		thruster.key = key;
		thruster.thrust_factor = 1.;
		
		vehicle_controller.AddThruster(thruster);
	}

	void ConstructVehicle(sim::Entity & entity, geom::rel::Sphere3 const & sphere)
	{
		ConstructBall(entity, sphere, gfx::Color4f::White());

		auto& controller = ref(new sim::VehicleController(entity));
		entity.SetController(& controller);

		AddThruster(controller, sim::Vector3(.5, -.8f, .5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
		AddThruster(controller, sim::Vector3(.5, -.8f, -.5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
		AddThruster(controller, sim::Vector3(-.5, -.8f, .5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
		AddThruster(controller, sim::Vector3(-.5, -.8f, -.5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);

	}
}

sim::EntityHandle SpawnBall(const sim::Vector3 & position, gfx::Color4f color)
{
	// ball
	auto ball = sim::EntityHandle::CreateHandle();

	ball.Call([position, color] (sim::Entity & ball) {
		geom::rel::Sphere3 sphere(position, geom::rel::Scalar(std::exp(- GetRandomUnit() * 2)));
		ConstructBall(ball, sphere, color);
	});

	return ball;
}

sim::EntityHandle SpawnBox(const sim::Vector3 & position, gfx::Color4f color)
{
	// box
	auto box = sim::EntityHandle::CreateHandle();

	box.Call([position, color] (sim::Entity & box) {
		geom::rel::Vector3 size(geom::rel::Scalar(std::exp(GetRandomUnit() * -2.)),
					 geom::rel::Scalar(std::exp(GetRandomUnit() * -2.)),
					 geom::rel::Scalar(std::exp(GetRandomUnit() * -2.)));
		ConstructBox(box, position, size, color);
	});

	return box;
}

sim::EntityHandle SpawnObserver(const sim::Vector3 & position)
{
	auto observer = sim::EntityHandle::CreateHandle();

	observer.Call([position] (sim::Entity & observer) {
		ConstructObserver(observer, position);
	});

	return observer;
}

sim::EntityHandle SpawnPlanet(const sim::Sphere3 & sphere, int random_seed, int num_craters)
{
	auto handle = sim::EntityHandle::CreateHandle();

	handle.Call([sphere, random_seed, num_craters] (sim::Entity & entity) {
		auto & engine = entity.GetEngine();

		// controller
		auto& controller = ref(new sim::PlanetController(entity, sphere, random_seed, num_craters));
		auto& formation = controller.GetFormation();
		entity.SetController(& controller);

		// body
		physics::Engine & physics_engine = engine.GetPhysicsEngine();
		auto body = new physics::PlanetBody(physics_engine, formation, physics::Scalar(sphere.radius));
		body->SetPosition(geom::Cast<physics::Scalar>(sphere.center));
		entity.SetLocation(body);

		// register with the renderer
#if defined(RENDER_SEA)
		gfx::Scalar sea_level = _radius_mean;
#else
		gfx::Scalar sea_level = 0;
#endif
		
		auto branch_node = gfx::BranchNodeHandle::CreateHandle(gfx::Transformation::Matrix44::Identity());
		auto model = gfx::PlanetHandle::CreateHandle(sea_level);
		gfx::Daemon::Call([model, branch_node] (gfx::Engine & engine) {
			engine.OnSetParent(model.GetUid(), branch_node.GetUid());
		});
		entity.SetModel(branch_node);
		controller.SetModel(model);
	});

	return handle;
}

gfx::ObjectHandle SpawnSkybox()
{
	auto skybox = sim::FirmamentHandle::CreateHandle();

	gfx::Daemon::Call([skybox] (gfx::Engine & engine) {
		engine.OnSetParent(skybox.GetUid(), gfx::Uid());
	});

	return skybox;
}

sim::EntityHandle SpawnStar()
{
	auto sun = sim::EntityHandle::CreateHandle();

	sun.Call([] (sim::Entity & sun) {
		// physics
		geom::rel::Vector3 position(65062512.f, 75939904.f, 0.f);
		auto location = new physics::FixedLocation(sim::Transformation(position));
		sun.SetLocation(location);

		// graphics
	        gfx::Color4f color(gfx::Color4f(1.f,.95f,.9f) * 7500000000000000.f);
	        auto light = sim::AddModelWithTransform<gfx::Light>(color);
		sun.SetModel(light);
	});

	return sun;
}

sim::EntityHandle SpawnVehicle(sim::Vector3 const & position)
{
	auto vehicle = sim::EntityHandle::CreateHandle();

	geom::rel::Sphere3 sphere;
	sphere.center = geom::Cast<float>(position);
	sphere.radius = 1.;

	vehicle.Call([sphere] (sim::Entity & vehicle) {
		ConstructVehicle(vehicle, sphere);
	});

	return vehicle;
}
