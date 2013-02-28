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
#include "gfx/object/Skybox.h"

#include "geom/origin.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"

namespace gfx 
{ 
	DECLARE_CLASS_HANDLE(Ball); // gfx::BallHandle
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
		gfx::Transformation local_transformation(spawn_pos, gfx::Transformation::Matrix33::Identity(), size * .5f);
		auto model = gfx::BoxHandle::CreateHandle(local_transformation, color);
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
		gfx::Transformation local_transformation(sphere.center, gfx::Transformation::Matrix33::Identity(), sphere.radius);
		gfx::ObjectHandle model = gfx::BallHandle::CreateHandle(local_transformation, color);
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

	void DrawStarsSlow(gfx::Skybox & skybox, int box_edge_size, int num_stars)
	{
		for (int axis = 0; axis < 3; ++ axis)
		{
			int x_axis = TriMod(axis + 1);
			int y_axis = TriMod(axis + 2);
			int z_axis = axis;
			for (int pole = 0; pole < 2; ++ pole)
			{
				gfx::Image side;
				side.Create(geom::Vector2i(box_edge_size, box_edge_size));
				side.Clear(gfx::Color4b::Black());

				geom::Vector2i pos;
				geom::Vector2f f_pos;
				for (pos.x = 0; pos.x < box_edge_size; ++ pos.x)
				{
					f_pos.x = ((static_cast<float>(pos.x) + .5f) / (static_cast<float>(box_edge_size) - .0f)) - .5f;

					for (pos.y = 0; pos.y < box_edge_size; ++ pos.y)
					{
						f_pos.y = ((static_cast<float>(pos.y) + .5f) / (static_cast<float>(box_edge_size) - .0f)) - .5f;

						double intensity = 0;
						Random random (1);

						geom::Sphere<float, 3> star;
						float const * axes = star.center.GetAxes();
	//					float & star_x = axes[x_axis];
	//					float & star_y = axes[y_axis];
						float const & star_z = axes[z_axis];

						geom::Vector3f line_direction;
						float * line_axes = line_direction.GetAxes();
						float & line_x = line_axes[x_axis];
						float & line_y = line_axes[y_axis];
						float & line_z = line_axes[z_axis];

						for (int i = num_stars; i; -- i)
						{
							star.center.x = random.GetUnitInclusive<float>() - .5f;
							star.center.y = random.GetUnitInclusive<float>() - .5f;
							star.center.z = random.GetUnitInclusive<float>() - .5f;
							star.radius = Square(random.GetUnitInclusive<float>());

							float w = star_z;
							if ((w > 0) != (pole != 0) || w == 0)
							{
								continue;
							}

							//float w_co = .5f / w;
							//geom::Vector2f uv(star_x * w_co, star_y * w_co);

							line_x = f_pos.x;
							line_y = f_pos.y;
							line_z = (w > 0) ? .5f : -.5f;
							Normalize(line_direction);

							float dp = DotProduct(star.center, line_direction);
							float a_sq = LengthSq(star.center) - Square(dp);

							intensity += (double)star.radius / (double)a_sq;
						}

						float comp = static_cast<float>(std::min(1., .0002 * intensity));
						gfx::Color4f c (comp);
						side.SetPixel(pos, c);
					}
				}

				skybox.SetSide(axis, pole, side);
			}
		}
	}

	void DrawStar(gfx::Image & side, geom::Vector2f const & uv, float r)
	{
		auto size = side.GetSize();
		geom::Vector2i pos;

		pos.x = static_cast<int>((uv.x + .5f) * size.x);
		if (pos.x < 0 || pos.x >= size.x) {
			return;
		}
	
		pos.y = static_cast<int>((uv.y + .5f) * size.y);
		if (pos.y < 0 || pos.y >= size.y) {
			return;
		}
	
		float comp = std::min(1.f, r);
		gfx::Color4f c (comp);
		side.SetPixel(pos, c);
	}

	void DrawStarsFast(gfx::Skybox & skybox, int box_edge_size, int num_stars)
	{
		for (int axis = 0; axis < 3; ++ axis)
		{
			for (int pole = 0; pole < 2; ++ pole)
			{
				gfx::Image side;
				side.Create(geom::Vector2i(box_edge_size, box_edge_size));
				side.Clear(gfx::Color4b::Black());
			
				Random random (1);
				for (int i = num_stars; i; -- i)
				{
					geom::Sphere<float, 3> star;
					//random.GetGaussians(star.center.x, star.center.y);
					//random.GetGaussians(star.center.z, star.radius);
					//star.radius = Abs(star.radius);
					star.center.x = random.GetUnitInclusive<float>() - .5f;
					star.center.y = random.GetUnitInclusive<float>() - .5f;
					star.center.z = random.GetUnitInclusive<float>() - .5f;
					star.radius = Square(random.GetUnitInclusive<float>());
				
					float const * axes = star.center.GetAxes();
					float w = axes[axis];
					if ((w > 0) != (pole != 0) || w == 0)
					{
						continue;
					}
				
					float w_co = .5f / w;
				
					geom::Vector2f uv(axes[TriMod(axis + 1)] * w_co, axes[TriMod(axis + 2)] * w_co);
					float radius = star.radius * Abs(w_co);
					DrawStar(side, uv, radius);
				}
			
				skybox.SetSide(axis, pole, side);
			}
		}
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
		
		auto model = gfx::PlanetHandle::CreateHandle(gfx::Transformation(sphere.center), sea_level);
		entity.SetModel(model);
	});

	return handle;
}

gfx::ObjectHandle SpawnSkybox()
{
	auto skybox = gfx::SkyboxHandle::CreateHandle();
	skybox.Call([] (gfx::Skybox & skybox) {
		//DrawStarsSlow(skybox, 256, 100);
		DrawStarsFast(skybox, 512, 20000);
	});

	return skybox;
}

sim::EntityHandle SpawnStar()
{
	auto sun = sim::EntityHandle::CreateHandle();

	sun.Call([] (sim::Entity & sun) {
		// physics
		geom::rel::Vector3 position(65062512.f, 75939904.f, 0.f);
		sim::Transformation transformation(position);
		auto location = new physics::FixedLocation(transformation);
		sun.SetLocation(location);

		// graphics
		gfx::Color4f color(gfx::Color4f(1.f,.95f,.9f) * 7500000000000000.f);
		auto light = gfx::LightHandle::CreateHandle(transformation, color);
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
