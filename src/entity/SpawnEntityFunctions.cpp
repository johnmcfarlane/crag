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
#include "entity/sim/HoverThruster.h"
#include "entity/sim/MouseObserverController.h"
#include "entity/sim/PlanetController.h"
#include "entity/sim/RoverThruster.h"
#include "entity/sim/TouchObserverController.h"
#include "entity/sim/VehicleController.h"
#include "entity/sim/VernierThruster.h"

#include "sim/Engine.h"
#include "sim/Entity.h"
#include "sim/EntityFunctions.h"

#include "physics/BoxBody.h"
#include "physics/Engine.h"
#include "physics/GhostBody.h"
#include "physics/MeshBody.h"
#include "physics/PassiveLocation.h"
#include "physics/SphericalBody.h"

#include "form/Engine.h"

#include "gfx/axes.h"
#include "gfx/Engine.h"
#include "gfx/object/Ball.h"
#include "gfx/object/Light.h"
#include "gfx/object/MeshObject.h"

#include "geom/origin.h"

#include "core/app.h"
#include "core/ConfigEntry.h"
#include "core/ResourceManager.h"

#if defined(CRAG_USE_MOUSE)
CONFIG_DEFINE (observer_use_touch, bool, false);
#elif defined(CRAG_USE_TOUCH)
CONFIG_DEFINE (observer_use_touch, bool, true);
#endif
CONFIG_DEFINE (observer_physics, bool, false);

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

	CONFIG_DEFINE (camera_radius, float, .5);
	CONFIG_DEFINE (camera_density, float, 1);
	
	CONFIG_DEFINE (camera_linear_damping, physics::Scalar, 0.5f);
	CONFIG_DEFINE (camera_angular_damping, physics::Scalar, 0.05f);

	CONFIG_DEFINE (ship_linear_damping, physics::Scalar, 0.05f);
	CONFIG_DEFINE (ship_angular_damping, physics::Scalar, 0.15f);
	CONFIG_DEFINE (ship_upward_thrust, physics::Scalar, 0.25f);
	CONFIG_DEFINE (ship_upward_thrust_gradient, physics::Scalar, 0.75f);
	CONFIG_DEFINE (ship_forward_thrust, physics::Scalar, 10.0f);

	////////////////////////////////////////////////////////////////////////////////
	// function definitions
	
	template <typename IndexType>
	gfx::Mesh<gfx::PlainVertex, IndexType> GenerateShipMesh()
	{
		// ship mesh
		gfx::Mesh<gfx::PlainVertex, IndexType> mesh;
		
		// add vertices
		auto & vertices = mesh.GetVertices();
		vertices.reserve(5);
		vertices.push_back(gfx::PlainVertex { sim::Vector3(0.f, 0.f, 1.f) });
		vertices.push_back(gfx::PlainVertex { sim::Vector3(-1.f, 0.f, -1.f) });
		vertices.push_back(gfx::PlainVertex { sim::Vector3(1.f, 0.f, -1.f) });
		vertices.push_back(gfx::PlainVertex { sim::Vector3(0.f, -.25f, -1.f) });
		vertices.push_back(gfx::PlainVertex { sim::Vector3(0.f, .25f, -1.f) });
		ASSERT(vertices.size() == vertices.capacity());

		// add faces
		auto & indices = mesh.GetIndices();
		indices.reserve(18);
		auto add_face = [& indices] (int a, int b, int c)
		{
			indices.push_back(a);
			indices.push_back(b);
			indices.push_back(c);
		};
		add_face(0, 1, 4);
		add_face(0, 4, 2);
		add_face(0, 2, 3);
		add_face(0, 3, 1);
		add_face(1, 3, 4);
		add_face(2, 4, 3);
		ASSERT(indices.size() == indices.capacity());
		
		// translate centroid to origin
		auto centroid = CalculateCentroidAndVolume(mesh);
		for (auto & vertex : vertices)
		{
			vertex.pos -= centroid.first;
		}
		
		// return result
		CRAG_VERIFY(mesh);
		return mesh;
	}
	
	// given a Mesh comprising unique vertex entries,
	// generates a GPU-friendly mesh with flat shading
	gfx::LitMesh GenerateFlatLitMesh(physics::Mesh const & source_mesh, gfx::Color4f const & color = gfx::Color4f::White())
	{
		auto & source_vertices = source_mesh.GetVertices();
		auto & source_indices = source_mesh.GetIndices();
		auto num_source_indices = source_indices.size();

		gfx::LitMesh destination_mesh;

		auto & destination_vertices = destination_mesh.GetVertices();
		auto & destination_indices = destination_mesh.GetIndices();
		
		destination_vertices.reserve(num_source_indices);
		destination_indices.reserve(num_source_indices);

		for (auto index_index = 0u; index_index != num_source_indices; index_index += 3)
		{
			gfx::Triangle3 source_triangle;
			for (int i = 0; i != 3; ++ i)
			{
				auto vert_index = source_indices[index_index + i];
				source_triangle.points[i] = source_vertices[vert_index].pos;
			}

			auto source_plane = geom::MakePlane(source_triangle);
			
			for (auto const & vertex : source_triangle.points)
			{
				destination_indices.push_back(destination_vertices.size());
				destination_vertices.push_back(gfx::LitVertex({ vertex, source_plane.normal, color, 0.f }));
			}
		}
		
		CRAG_VERIFY_EQUAL(source_vertices.size(), source_vertices.capacity());
		CRAG_VERIFY_EQUAL(source_indices.size(), source_indices.capacity());

		CRAG_VERIFY(destination_mesh);
		return destination_mesh;
	}
	
	void ConstructBox(sim::Entity & box, geom::rel::Vector3 spawn_pos, sim::Vector3 const & velocity, geom::rel::Vector3 size, gfx::Color4f color)
	{
		auto & resource_manager = crag::core::ResourceManager::Get();

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
		auto lit_vbo = resource_manager.GetHandle<gfx::LitVboResource>("CuboidVbo");
		auto plain_mesh = resource_manager.GetHandle<gfx::PlainMesh>("CuboidPlainMesh");
		auto model = gfx::MeshObjectHandle::CreateHandle(local_transformation, color, size, lit_vbo, plain_mesh);
		box.SetModel(model);
	}

	void ConstructBody(sim::Entity & entity, geom::rel::Vector3 const & position, sim::Vector3 const & velocity, physics::Mass m, float linear_damping, float angular_damping)
	{
		sim::Engine & engine = entity.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto & body = * new physics::GhostBody(position, velocity, physics_engine);
		
		// setting the mass of a shapeless body is somewhat nonsensical
		body.SetMass(m);
		body.SetLinearDamping(linear_damping);
		body.SetAngularDamping(angular_damping);
		entity.SetLocation(& body);
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
			if (observer_physics)
			{
				ConstructSphericalBody(observer, geom::rel::Sphere3(position, observer_radius), sim::Vector3::Zero(), observer_density, observer_linear_damping, observer_angular_damping);
			}
			else
			{
				physics::Mass m;
				dMassSetSphere(& m, observer_density, observer_radius);
				ConstructBody(observer, position, sim::Vector3::Zero(), m, observer_linear_damping, observer_angular_damping);
			}
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

	void ConstructCamera(sim::Entity & camera, sim::Vector3 const & position, sim::EntityHandle subject_handle)
	{
		// physics
		ConstructSphericalBody(camera, geom::rel::Sphere3(position, camera_radius), sim::Vector3::Zero(), camera_density, camera_linear_damping, camera_angular_damping);

		// controller
		camera.SetController(new sim::CameraController(camera, subject_handle));
	}

	void AddHoverThruster(sim::VehicleController & controller, sim::Vector3 const & position, sim::Scalar distance)
	{
		auto & entity = controller.GetEntity();
		auto thruster = new sim::HoverThruster(entity, position, distance);
		controller.AddThruster(thruster);
	}

	void AddRoverThruster(sim::VehicleController & controller, sim::Ray3 const & ray, SDL_Scancode key, bool invert = false)
	{
		auto & entity = controller.GetEntity();
		auto activation_callback = [key, invert] ()
		{
			return (app::IsKeyDown(key) != invert) ? 1.f : 0.f;
		};
		auto * thruster = new sim::RoverThruster(entity, ray, activation_callback);
		controller.AddThruster(thruster);
	}

	void AddVernierThruster(sim::VehicleController & controller, sim::Ray3 const & ray)
	{
		auto & entity = controller.GetEntity();
		auto * thruster = new sim::VernierThruster(entity, ray);
		controller.AddThruster(thruster);
	}

	void ConstructRover(sim::Entity & entity, geom::rel::Sphere3 const & sphere)
	{
		ConstructBall(entity, sphere, sim::Vector3::Zero(), gfx::Color4f::White());

		auto& controller = ref(new sim::VehicleController(entity));
		entity.SetController(& controller);

		AddRoverThruster(controller, sim::Ray3(sim::Vector3(.5, -.8f, .5), sim::Vector3(0, 15, 0)), SDL_SCANCODE_H);
		AddRoverThruster(controller, sim::Ray3(sim::Vector3(.5, -.8f, -.5), sim::Vector3(0, 15, 0)), SDL_SCANCODE_H);
		AddRoverThruster(controller, sim::Ray3(sim::Vector3(-.5, -.8f, .5), sim::Vector3(0, 15, 0)), SDL_SCANCODE_H);
		AddRoverThruster(controller, sim::Ray3(sim::Vector3(-.5, -.8f, -.5), sim::Vector3(0, 15, 0)), SDL_SCANCODE_H);
	}
	
	void ConstructShip(sim::Entity & entity, sim::Vector3 const & position)
	{
		// resources
		auto & resource_manager = crag::core::ResourceManager::Get();
		resource_manager.Register<physics::Mesh>("ShipPhysicsMesh", [] ()
		{
			return GenerateShipMesh<dTriIndex>();
		});
		resource_manager.Register<gfx::PlainMesh>("ShipPlainMesh", [] ()
		{
			return GenerateShipMesh<gfx::ElementIndex>();
		});
		resource_manager.Register<gfx::LitMesh>("ShipLitMesh", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto physics_mesh = resource_manager.GetHandle<physics::Mesh>("ShipPhysicsMesh");
			return GenerateFlatLitMesh(* physics_mesh);
		});
		
		auto lit_mesh_handle = resource_manager.GetHandle<gfx::LitMesh>("ShipLitMesh");
		resource_manager.Register<gfx::LitVboResource>("ShipVbo", [lit_mesh_handle] ()
		{
			return gfx::LitVboResource(* lit_mesh_handle);
		});

		// physics
		sim::Engine & sim_engine = entity.GetEngine();
		physics::Engine & physics_engine = sim_engine.GetPhysicsEngine();

		auto velocity = sim::Vector3::Zero();
		auto physics_mesh = resource_manager.GetHandle<physics::Mesh>("ShipPhysicsMesh");
		auto & body = * new physics::MeshBody(position, & velocity, physics_engine, * physics_mesh);
		body.SetLinearDamping(ship_linear_damping);
		body.SetAngularDamping(ship_angular_damping);
		entity.SetLocation(& body);

		// graphics
		gfx::Transformation local_transformation(position, gfx::Transformation::Matrix33::Identity());
		gfx::Color4f const & color = gfx::Color4f::White();
		gfx::Vector3 scale(1.f, 1.f, 1.f);
		auto lit_vbo = resource_manager.GetHandle<gfx::LitVboResource>("ShipVbo");
		auto plain_mesh = resource_manager.GetHandle<gfx::PlainMesh>("ShipPlainMesh");
		gfx::ObjectHandle model_handle = gfx::MeshObjectHandle::CreateHandle(local_transformation, color, scale, lit_vbo, plain_mesh);
		entity.SetModel(model_handle);

		gfx::ObjectHandle beam_handle = gfx::LightHandle::CreateHandle(gfx::Transformation(), gfx::Color4f::Red() * 100000.f, gfx::LightType::beam);
		gfx::Daemon::Call([beam_handle, model_handle] (gfx::Engine & engine) {
			engine.OnSetParent(beam_handle.GetUid(), model_handle.GetUid());
		});

		// controller
		auto & controller = ref(new sim::VehicleController(entity));
		entity.SetController(& controller);

		// add a single thruster
		auto add_thruster = [&] (sim::Ray3 const & ray, SDL_Scancode key, bool invert = false)
		{
			if (key == SDL_SCANCODE_UNKNOWN)
			{
				AddVernierThruster(controller, ray);
			}
			else
			{
				AddRoverThruster(controller, ray, key, invert);
			}
		};
		
		// add two complimentary thrusters
		auto add_thrusters = [&] (sim::Ray3 ray, SDL_Scancode first_key, int axis, SDL_Scancode second_key, bool invert = false)
		{
			add_thruster(ray, first_key, invert);
			ray.position[axis] *= -1.f;
			ray.direction[axis] *= -1.f;
			add_thruster(ray, second_key, invert);
		};
		
		auto forward = sim::Vector3(0, 0, ship_forward_thrust);

		add_thrusters(sim::Ray3(sim::Vector3(0.f, 0.f, 1.f), geom::Resized(sim::Vector3(0.f, 1.f, - ship_upward_thrust_gradient), ship_upward_thrust)), SDL_SCANCODE_UNKNOWN, 2, SDL_SCANCODE_UNKNOWN);
		add_thrusters(sim::Ray3(sim::Vector3(1., 0.f, 0), geom::Resized(sim::Vector3(- ship_upward_thrust_gradient, 1.f, 0.f), ship_upward_thrust)), SDL_SCANCODE_UNKNOWN, 0, SDL_SCANCODE_UNKNOWN);
		add_thrusters(sim::Ray3(sim::Vector3(.25f, 0.f, -.525f), forward * .5f), SDL_SCANCODE_RIGHT, 0, SDL_SCANCODE_LEFT, true);
		
		AddHoverThruster(controller, sim::Vector3(0.f, -.25f, 0.f), -.1f);
		AddHoverThruster(controller, sim::Vector3(0.f, .25f, 0.f), .1f);
	}
}

sim::EntityHandle SpawnBall(sim::Sphere3 const & sphere, sim::Vector3 const & velocity, gfx::Color4f color)
{
	ASSERT(color.a = 1);
	
	// ball
	auto ball = sim::EntityHandle::CreateHandle();

	ball.Call([sphere, velocity, color] (sim::Entity & entity) {
		ConstructBall(entity, sphere, velocity, color);
	});

	return ball;
}

sim::EntityHandle SpawnBox(sim::Vector3 const & position, sim::Vector3 const & velocity, sim::Vector3 const & size, gfx::Color4f color)
{
	ASSERT(color.a = 1);
	
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

sim::EntityHandle SpawnCamera(sim::Vector3 const & position, sim::EntityHandle subject)
{
	auto camera = sim::EntityHandle::CreateHandle();

	camera.Call([position, subject] (sim::Entity & entity) {
		ConstructCamera(entity, position, subject);
	});

	return camera;
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

// assumes origin is zero
sim::EntityHandle SpawnStar(geom::abs::Sphere3 const & volume, gfx::Color4f const & color)
{
	auto sun = sim::EntityHandle::CreateHandle();

	sun.Call([volume, color] (sim::Entity & entity) {
		// physics
		sim::Transformation transformation(geom::Cast<sim::Scalar>(volume.center));
		auto location = new physics::PassiveLocation(transformation);
		entity.SetLocation(location);

		// graphics
		auto light = gfx::LightHandle::CreateHandle(transformation, color, gfx::LightType::shadow);
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

sim::EntityHandle SpawnShip(sim::Vector3 const & position)
{
	auto ship = sim::EntityHandle::CreateHandle();

	ship.Call([position] (sim::Entity & entity) {
		ConstructShip(entity, position);
	});

	return ship;
}
