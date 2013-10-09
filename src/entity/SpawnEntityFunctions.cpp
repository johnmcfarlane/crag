//
//  SpawnEntityFunctions.cpp
//  crag
//
//  Created by John McFarlane on 2013-02-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

//#include "SpawnEntityFunctions.h"

#include "observer/MouseObserverController.h"
#include "observer/TouchObserverController.h"

#include "planet/sim/PlanetController.h"
#include "planet/physics/PlanetBody.h"
#include "planet/gfx/Planet.h"

#include "vehicle/sim/RoverThruster.h"
#include "vehicle/sim/VehicleController.h"

#include "sim/Engine.h"
#include "sim/Entity.h"
#include "sim/EntityFunctions.h"

#include "physics/BoxBody.h"
#include "physics/Engine.h"
#include "physics/PassiveLocation.h"
#include "physics/SphericalBody.h"

#include "form/Scene.h"

#include "gfx/Engine.h"
#include "gfx/object/Ball.h"
#include "gfx/object/Box.h"
#include "gfx/object/Light.h"

#include "geom/origin.h"

#include "core/app.h"
#include "core/ConfigEntry.h"

#if defined(CRAG_USE_MOUSE)
CONFIG_DEFINE (observer_use_touch, bool, false);
#elif defined(CRAG_USE_TOUCH)
CONFIG_DEFINE (observer_use_touch, bool, true);
#endif

namespace gfx 
{ 
	DECLARE_CLASS_HANDLE(Box); // gfx::BoxHandle
	DECLARE_CLASS_HANDLE(Light); // gfx::LightHandle
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
	
	void ConstructBox(sim::Entity & box, geom::rel::Vector3 spawn_pos, sim::Vector3 const & velocity, geom::rel::Vector3 size, gfx::Color4f color)
	{
		// physics
		sim::Engine & engine = box.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto & body = * new physics::BoxBody(spawn_pos, & velocity, physics_engine, size);
		body.SetDensity(box_density);
		body.SetLinearDamping(box_linear_damping);
		body.SetAngularDamping(box_angular_damping);
		box.SetLocation(& body);

		// graphics
		gfx::Transformation local_transformation(spawn_pos, gfx::Transformation::Matrix33::Identity());
		auto model = gfx::BoxHandle::CreateHandle(local_transformation, size, color);
		box.SetModel(model);
	}

	void ConstructSphericalBody(sim::Entity & entity, geom::rel::Sphere3 const & sphere, sim::Vector3 const & velocity, float density, float linear_damping, float angular_damping)
	{
		sim::Engine & engine = entity.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto & body = * new physics::SphericalBody(sphere.center, & velocity, physics_engine, sphere.radius);
		body.SetDensity(density);
		body.SetLinearDamping(linear_damping);
		body.SetAngularDamping(angular_damping);
		entity.SetLocation(& body);
	}

	void ConstructBall(sim::Entity & ball, geom::rel::Sphere3 sphere, sim::Vector3 const & velocity, gfx::Color4f color)
	{
		// physics
		ConstructSphericalBody(ball, sphere, velocity, ball_density, ball_linear_damping, ball_angular_damping);

		// graphics
		gfx::Transformation local_transformation(sphere.center, gfx::Transformation::Matrix33::Identity());
		gfx::ObjectHandle model = gfx::BallHandle::CreateHandle(local_transformation, sphere.radius, color);
		ball.SetModel(model);
	}

	void ConstructObserver(sim::Entity & observer, sim::Vector3 const & position)
	{
		// physics
		if (! observer_use_touch)
		{
			ConstructSphericalBody(observer, geom::rel::Sphere3(position, observer_radius), sim::Vector3::Zero(), observer_density, observer_linear_damping, observer_angular_damping);
		}

		// controller
		auto controller = [&] () -> sim::Controller *
		{
			if (! observer_use_touch)
			{
				if (SDL_SetRelativeMouseMode(SDL_TRUE) == 0)
				{
					return new sim::MouseObserverController(observer);
				}
				else
				{
					// Linux requires libxi-dev to be installed for this to succeed.
					DEBUG_MESSAGE("Failed to set relative mouse mode.");
				}
			}

			return new sim::TouchObserverController(observer, position);
		} ();
		
		observer.SetController(controller);

#if defined(OBSERVER_LIGHT)
		// register light with the renderer
		gfx::Light * light = new gfx::Light(observer_light_color);
		_light_uid = AddModelWithTransform(* light);
#endif
	}

	void AddRoverThruster(sim::VehicleController & controller, sim::Vector3 const & position, sim::Vector3 const & direction, SDL_Scancode key)
	{
		auto & entity = controller.GetEntity();
		sim::Ray3 ray(position, direction);
		auto thruster = new sim::RoverThruster(entity, ray, key);
		controller.AddThruster(thruster);
	}

	void ConstructRover(sim::Entity & entity, geom::rel::Sphere3 const & sphere)
	{
		ConstructBall(entity, sphere, sim::Vector3::Zero(), gfx::Color4f::White());

		auto& controller = ref(new sim::VehicleController(entity));
		entity.SetController(& controller);

		AddRoverThruster(controller, sim::Vector3(.5, -.8f, .5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
		AddRoverThruster(controller, sim::Vector3(.5, -.8f, -.5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
		AddRoverThruster(controller, sim::Vector3(-.5, -.8f, .5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
		AddRoverThruster(controller, sim::Vector3(-.5, -.8f, -.5), sim::Vector3(0, 5, 0), SDL_SCANCODE_H);
	}
}

sim::EntityHandle SpawnBall(sim::Sphere3 const & sphere, sim::Vector3 const & velocity, gfx::Color4f color)
{
	// ball
	auto ball = sim::EntityHandle::CreateHandle();

	ball.Call([sphere, velocity, color] (sim::Entity & entity) {
		ConstructBall(entity, sphere, velocity, color);
	});

	return ball;
}

sim::EntityHandle SpawnBox(sim::Vector3 const & position, sim::Vector3 const & velocity, sim::Vector3 const & size, gfx::Color4f color)
{
	// box
	auto box = sim::EntityHandle::CreateHandle();

	box.Call([position, velocity, size, color] (sim::Entity & entity) {
		ConstructBox(entity, position, velocity, size, color);
	});

	return box;
}

sim::EntityHandle SpawnObserver(const sim::Vector3 & position)
{
	auto observer = sim::EntityHandle::CreateHandle();

	observer.Call([position] (sim::Entity & entity) {
		ConstructObserver(entity, position);
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
		auto const * polyhedron = engine.GetScene().GetPolyhedron(formation);
		if (polyhedron)
		{
			auto body = new physics::PlanetBody(sphere.center, physics_engine, * polyhedron, physics::Scalar(sphere.radius));
			entity.SetLocation(body);
		}
		else
		{
			DEBUG_BREAK("missing formation polyhedron");
		}

		// register with the renderer
#if defined(RENDER_SEA)
		gfx::Scalar sea_level = _radius_mean;
#else
		gfx::Scalar sea_level = 0;
#endif
		
		auto model = gfx::PlanetHandle::CreateHandle(gfx::Transformation(sphere.center), sea_level);
		entity.SetModel(model);
	});

	return handle;
}

sim::EntityHandle SpawnStar()
{
	auto sun = sim::EntityHandle::CreateHandle();

	sun.Call([] (sim::Entity & entity) {
		// physics
		geom::rel::Vector3 position(65062512.f, 75939904.f, 0.f);
		sim::Transformation transformation(position);
		auto location = new physics::PassiveLocation(transformation);
		entity.SetLocation(location);

		// graphics
		gfx::Color4f color(gfx::Color4f(1.f,.95f,.9f) * 7500000000000000.f);
		auto light = gfx::LightHandle::CreateHandle(transformation, color);
		entity.SetModel(light);
	});

	return sun;
}

sim::EntityHandle SpawnRover(sim::Vector3 const & position)
{
	auto vehicle = sim::EntityHandle::CreateHandle();

	geom::rel::Sphere3 sphere;
	sphere.center = geom::Cast<float>(position);
	sphere.radius = 1.;

	vehicle.Call([sphere] (sim::Entity & entity) {
		ConstructRover(entity, sphere);
	});

	return vehicle;
}
