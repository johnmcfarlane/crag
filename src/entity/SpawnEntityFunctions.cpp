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

#include "entity/sim/CameraController.h"

#include "entity/gfx/Planet.h"
#include "entity/physics/PlanetBody.h"
#include "entity/sim/PlanetController.h"

#include "sim/Engine.h"
#include "sim/Entity.h"
#include "sim/Model.h"

#include "physics/BoxBody.h"
#include "physics/PassiveLocation.h"

#include "form/Scene.h"

#include "gfx/Color.h"
#include "gfx/Engine.h"
#include "gfx/object/Ball.h"
#include "gfx/object/Light.h"
#include "gfx/object/MeshObject.h"

#include "core/ConfigEntry.h"

using namespace sim;

namespace gfx 
{ 
	DECLARE_CLASS_HANDLE(Box); // gfx::BoxHandle
	DECLARE_CLASS_HANDLE(Light); // gfx::LightHandle
}

namespace
{
	////////////////////////////////////////////////////////////////////////////////
	// Config values

	CONFIG_DEFINE(box_density, 1.f);

	CONFIG_DEFINE(ball_density, 1.f);
	CONFIG_DEFINE(ball_linear_damping, 0.01f);

	CONFIG_DEFINE(camera_radius, .5f);
	CONFIG_DEFINE(camera_density, 1.f);
	CONFIG_DEFINE(camera_linear_damping, 0.5f);

	////////////////////////////////////////////////////////////////////////////////
	// local functions

	void ConstructBox(Entity & box, geom::rel::Vector3 spawn_pos, Vector3 const & velocity, geom::rel::Vector3 size, gfx::Color4f color)
	{
		// physics
		Engine & engine = box.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto body = std::unique_ptr<physics::BoxBody>(
			new physics::BoxBody(spawn_pos, & velocity, physics_engine, size));
		body->SetDensity(box_density);
		box.SetLocation(std::move(body));

		// graphics
		gfx::Transformation local_transformation(spawn_pos, gfx::Transformation::Matrix33::Identity());
		auto model_handle = gfx::MeshObjectHandle::Create(local_transformation, color, size, "CuboidVbo", "CuboidPlainMesh");
		box.SetModel(Entity::ModelPtr(new Model(model_handle, * box.GetLocation())));
	}

	void ConstructSphereBody(Entity & entity, geom::rel::Sphere3 const & sphere, Vector3 const & velocity, float density, float linear_damping)
	{
		Engine & engine = entity.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto body = std::unique_ptr<physics::SphereBody>(
			new physics::SphereBody(sphere.center, & velocity, physics_engine, sphere.radius));
		body->SetDensity(density);
		body->SetLinearDamping(linear_damping);
		
		entity.SetLocation(std::move(body));
	}

	void ConstructBall(Entity & ball, geom::rel::Sphere3 sphere, Vector3 const & velocity, gfx::Color4f color)
	{
		// physics
		ConstructSphereBody(ball, sphere, velocity, ball_density, ball_linear_damping);

		// graphics
		gfx::Transformation local_transformation(sphere.center, gfx::Transformation::Matrix33::Identity());
		gfx::ObjectHandle model_handle = gfx::BallHandle::Create(local_transformation, sphere.radius, color);
		ball.SetModel(Entity::ModelPtr(new Model(model_handle, * ball.GetLocation())));
	}

	void ConstructCamera(Entity & camera, Vector3 const & position, EntityHandle subject_handle)
	{
		// physics
		ConstructSphereBody(camera, geom::rel::Sphere3(position, camera_radius), Vector3::Zero(), camera_density, camera_linear_damping);

		// controller
		auto & engine = camera.GetEngine();
		auto const & subject = engine.GetObject(subject_handle);
		auto controller = std::unique_ptr<CameraController>(new CameraController(camera, subject));
		camera.SetController(std::move(controller));
	}
}

EntityHandle SpawnBall(Sphere3 const & sphere, Vector3 const & velocity, gfx::Color4f color)
{
	ASSERT(color.a = 1);

	// ball
	auto ball = EntityHandle::Create();

	ball.Call([sphere, velocity, color] (Entity & entity) {
		ConstructBall(entity, sphere, velocity, color);
	});

	return ball;
}

EntityHandle SpawnBox(Vector3 const & position, Vector3 const & velocity, Vector3 const & size, gfx::Color4f color)
{
	ASSERT(color.a = 1);
	
	// box
	auto box = EntityHandle::Create();

	box.Call([position, velocity, size, color] (Entity & entity) {
		ConstructBox(entity, position, velocity, size, color);
	});

	return box;
}

EntityHandle SpawnCamera(Vector3 const & position, EntityHandle subject)
{
	auto camera = EntityHandle::Create();

	camera.Call([position, subject] (Entity & entity) {
		ConstructCamera(entity, position, subject);
	});

	return camera;
}

EntityHandle SpawnPlanet(const Sphere3 & sphere, int random_seed, int num_craters)
{
	auto handle = EntityHandle::Create();

	handle.Call([sphere, random_seed, num_craters] (Entity & entity) {
		// create controller
		auto controller = std::unique_ptr<PlanetController>(
			new PlanetController(entity, sphere, random_seed, num_craters));

		// body
#if defined(CRAG_SIM_FORMATION_PHYSICS)
		auto & engine = entity.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();
		auto const & formation = controller->GetFormation();
		auto const * polyhedron = engine.GetScene().GetPolyhedron(formation);
		if (polyhedron)
		{
			auto body = std::unique_ptr<physics::PlanetBody>(
				new physics::PlanetBody(sphere.center, physics_engine, * polyhedron, physics::Scalar(sphere.radius)));
			entity.SetLocation(std::move(body));
		}
		else
		{
			DEBUG_BREAK("missing formation polyhedron");
		}
#else
		auto body = std::unique_ptr<physics::PassiveLocation>(new physics::PassiveLocation(sphere.center));
		entity.SetLocation(std::move(body));
#endif

		// transfer controller
		entity.SetController(std::move(controller));

		// register with the renderer
		auto model_handle = gfx::PlanetHandle::Create(gfx::Transformation(sphere.center));
		entity.SetModel(Entity::ModelPtr(new Model(model_handle, * entity.GetLocation())));
	});

	return handle;
}

// assumes origin is zero
EntityHandle SpawnStar(geom::uni::Sphere3 const & volume, gfx::Color4f const & color, bool casts_shadow)
{
	auto sun = EntityHandle::Create();

	sun.Call([volume, color, casts_shadow] (Entity & entity) {
		// physics
		Transformation transformation(static_cast<Vector3>(volume.center));
		auto location = std::unique_ptr<physics::PassiveLocation>(new physics::PassiveLocation(transformation));
		entity.SetLocation(std::move(location));

		// graphics
		auto light_handle = gfx::LightHandle::Create(
			transformation, color, 
			gfx::LightAttributes { gfx::LightResolution::vertex, gfx::LightType::point, casts_shadow });
		entity.SetModel(Entity::ModelPtr(new Model(light_handle, * entity.GetLocation())));
	});

	return sun;
}
