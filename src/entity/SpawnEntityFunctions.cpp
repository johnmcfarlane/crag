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

#include "physics/defs.h"
#include "physics/BoxBody.h"
#include "physics/Engine.h"
#include "physics/GhostBody.h"
#include "physics/PassiveLocation.h"
#include "physics/SphereBody.h"

#include "form/Scene.h"

#include "gfx/Color.h"
#include "gfx/Engine.h"
#include "gfx/LitVertex.h"
#include "gfx/PlainVertex.h"
#include "gfx/object/Ball.h"
#include "gfx/object/Light.h"
#include "gfx/object/MeshObject.h"

#include "core/ConfigEntry.h"
#include "core/ResourceManager.h"

using namespace std;
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

	CONFIG_DEFINE (box_density, physics::Scalar, 1);

	CONFIG_DEFINE (ball_density, float, 1);

	CONFIG_DEFINE (camera_radius, float, .5);
	CONFIG_DEFINE (camera_density, float, 1);
	
	CONFIG_DEFINE (camera_linear_damping, physics::Scalar, 0.5f);
	
	////////////////////////////////////////////////////////////////////////////////
	// local functions

	void ConstructBox(Entity & box, geom::rel::Vector3 spawn_pos, Vector3 const & velocity, geom::rel::Vector3 size, gfx::Color4f color)
	{
		auto & resource_manager = crag::core::ResourceManager::Get();

		// physics
		Engine & engine = box.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto body = make_shared<physics::BoxBody>(spawn_pos, & velocity, physics_engine, size);
		body->SetDensity(box_density);
		box.SetLocation(body);

		// graphics
		gfx::Transformation local_transformation(spawn_pos, gfx::Transformation::Matrix33::Identity());
		auto lit_vbo = resource_manager.GetHandle<gfx::VboResource>("CuboidVbo");
		auto plain_mesh = resource_manager.GetHandle<gfx::PlainMesh>("CuboidPlainMesh");
		auto model = gfx::MeshObjectHandle::Create(local_transformation, color, size, lit_vbo, plain_mesh);
		box.SetModel(model);
	}

	shared_ptr<physics::SphereBody> ConstructSphereBody(Entity & entity, geom::rel::Sphere3 const & sphere, Vector3 const & velocity, float density)
	{
		Engine & engine = entity.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto body = make_shared<physics::SphereBody>(sphere.center, & velocity, physics_engine, sphere.radius);
		body->SetDensity(density);
		entity.SetLocation(body);
		
		return body;
	}

	void ConstructBall(Entity & ball, geom::rel::Sphere3 sphere, Vector3 const & velocity, gfx::Color4f color)
	{
		// physics
		ConstructSphereBody(ball, sphere, velocity, ball_density);

		// graphics
		gfx::Transformation local_transformation(sphere.center, gfx::Transformation::Matrix33::Identity());
		gfx::ObjectHandle model = gfx::BallHandle::Create(local_transformation, sphere.radius, color);
		ball.SetModel(model);
	}

	void ConstructCamera(Entity & camera, Vector3 const & position, EntityHandle subject_handle)
	{
		// physics
		auto body = ConstructSphereBody(camera, geom::rel::Sphere3(position, camera_radius), Vector3::Zero(), camera_density);
		body->SetLinearDamping(camera_linear_damping);

		// controller
		auto & engine = camera.GetEngine();
		auto const & subject = engine.GetObject(subject_handle);
		auto controller = make_shared<CameraController>(camera, subject);
		camera.SetController(controller);
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
		auto & engine = entity.GetEngine();

		// controller
		auto controller = make_shared<PlanetController>(entity, sphere, random_seed, num_craters);
		auto& formation = controller->GetFormation();
		entity.SetController(controller);

		// body
		physics::Engine & physics_engine = engine.GetPhysicsEngine();
		auto const * polyhedron = engine.GetScene().GetPolyhedron(formation);
		if (polyhedron)
		{
			auto body = make_shared<physics::PlanetBody>(sphere.center, physics_engine, * polyhedron, physics::Scalar(sphere.radius));
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
		
		auto model = gfx::PlanetHandle::Create(gfx::Transformation(sphere.center), sea_level);
		entity.SetModel(model);
	});

	return handle;
}

// assumes origin is zero
EntityHandle SpawnStar(geom::abs::Sphere3 const & volume, gfx::Color4f const & color, bool casts_shadow)
{
	auto sun = EntityHandle::Create();

	sun.Call([volume, color, casts_shadow] (Entity & entity) {
		// physics
		Transformation transformation(geom::Cast<Scalar>(volume.center));
		auto location = make_shared<physics::PassiveLocation>(transformation);
		entity.SetLocation(location);

		// graphics
		auto light = gfx::LightHandle::Create(
			transformation, color, 
			gfx::LightAttributes { gfx::LightResolution::vertex, gfx::LightType::point, casts_shadow });
		entity.SetModel(light);
	});

	return sun;
}
