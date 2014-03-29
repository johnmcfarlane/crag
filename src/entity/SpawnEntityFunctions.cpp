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
#include "entity/sim/UfoController.h"
#include "entity/sim/TouchObserverController.h"
#include "entity/sim/VehicleController.h"
#include "entity/sim/VernierThruster.h"

#include "sim/Engine.h"
#include "sim/Entity.h"
#include "sim/EntityFunctions.h"

#include "physics/BoxBody.h"
#include "physics/CylinderBody.h"
#include "physics/Engine.h"
#include "physics/GhostBody.h"
#include "physics/MeshBody.h"
#include "physics/PassiveLocation.h"
#include "physics/SphereBody.h"

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

using namespace sim;

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

	CONFIG_DEFINE (ufo_height, physics::Scalar, .3f);
	CONFIG_DEFINE (ufo_radius, physics::Scalar, 1.f);
	CONFIG_DEFINE (ufo_linear_damping, physics::Scalar, 0.01f);
	CONFIG_DEFINE (ufo_angular_damping, physics::Scalar, 0.05f);
	CONFIG_DEFINE (ufo_stabilizer_thrust, physics::Scalar, .5f);
	CONFIG_DEFINE (ufo_stabilizer_distance, physics::Scalar, 0.2f);
	CONFIG_DEFINE (enable_beam, bool, false);

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
		vertices.push_back(gfx::PlainVertex { Vector3(0.f, 0.f, 1.f) });
		vertices.push_back(gfx::PlainVertex { Vector3(-1.f, 0.f, -1.f) });
		vertices.push_back(gfx::PlainVertex { Vector3(1.f, 0.f, -1.f) });
		vertices.push_back(gfx::PlainVertex { Vector3(0.f, -.25f, -1.f) });
		vertices.push_back(gfx::PlainVertex { Vector3(0.f, .25f, -1.f) });
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
		auto centroid = CalculateCentroidAndVolume(mesh).first;
		for (auto & vertex : vertices)
		{
			vertex.pos -= centroid;
		}
		
		// return result
		CRAG_VERIFY(mesh);
		return mesh;
	}
	
	template <typename IndexType>
	gfx::Mesh<gfx::PlainVertex, IndexType> GenerateUfoMesh(Scalar height, Scalar radius)
	{
		// ufo mesh
		gfx::Mesh<gfx::PlainVertex, IndexType> mesh;
		
		// add vertices
		constexpr auto num_sectors = 8;
		const auto inner_scale = geom::MakeVector(.5f, .5f, height * .5f);
		const auto outer_scale = geom::MakeVector(radius, radius, height * - .5f);
		auto & vertices = mesh.GetVertices();
		auto add_vertices = [& vertices, & inner_scale, & outer_scale] (Vector3 const & radial)
		{
			vertices.push_back(gfx::PlainVertex { radial * inner_scale });
			vertices.push_back(gfx::PlainVertex { radial * outer_scale });
		};
		vertices.reserve(num_sectors * 2 + 2);
		for (auto sector = 0; sector != num_sectors; ++ sector)
		{
			auto angle = float(PI * 2 * sector) / num_sectors;
			auto radial = geom::MakeVector(std::sin(angle), std::cos(angle), 1.f);
			add_vertices(radial);
		}
		add_vertices(Vector3(0.f, 0.f, 1.f));
		CRAG_VERIFY_EQUAL(vertices.size(), vertices.capacity());

		// add faces
		constexpr auto num_faces = num_sectors * 4;
		constexpr auto num_indices = num_faces * 3;
		constexpr auto inner_center = num_sectors * 2;
		constexpr auto outer_center = inner_center + 1;
		auto & indices = mesh.GetIndices();
		indices.reserve(num_indices);
		auto add_face = [& indices] (int a, int b, int c)
		{
			indices.push_back(a);
			indices.push_back(b);
			indices.push_back(c);
		};
		for (auto sector = 0; sector != num_sectors; ++ sector)
		{
			int inner_index[2] = { sector * 2, ((sector + 1) % num_sectors) * 2 };
			int outer_index[2] = { inner_index[0] + 1, inner_index[1] + 1 };
			add_face(inner_index[0], inner_index[1], inner_center);
			add_face(inner_index[1], inner_index[0], outer_index[0]);
			add_face(outer_index[0], outer_index[1], inner_index[1]);
			add_face(outer_index[1], outer_index[0], outer_center);
		}
		CRAG_VERIFY_EQUAL(indices.size(), indices.capacity());
		
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
	
	void ConstructBox(Entity & box, geom::rel::Vector3 spawn_pos, Vector3 const & velocity, geom::rel::Vector3 size, gfx::Color4f color)
	{
		auto & resource_manager = crag::core::ResourceManager::Get();

		// physics
		Engine & engine = box.GetEngine();
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

	void ConstructBody(Entity & entity, geom::rel::Vector3 const & position, Vector3 const & velocity, physics::Mass m, float linear_damping, float angular_damping)
	{
		Engine & engine = entity.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto & body = * new physics::GhostBody(position, velocity, physics_engine);
		
		// setting the mass of a shapeless body is somewhat nonsensical
		body.SetMass(m);
		body.SetLinearDamping(linear_damping);
		body.SetAngularDamping(angular_damping);
		entity.SetLocation(& body);
	}

	void ConstructSphereBody(Entity & entity, geom::rel::Sphere3 const & sphere, Vector3 const & velocity, float density, float linear_damping, float angular_damping)
	{
		Engine & engine = entity.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto & body = * new physics::SphereBody(sphere.center, & velocity, physics_engine, sphere.radius);
		body.SetDensity(density);
		body.SetLinearDamping(linear_damping);
		body.SetAngularDamping(angular_damping);
		entity.SetLocation(& body);
	}

	void ConstructBall(Entity & ball, geom::rel::Sphere3 sphere, Vector3 const & velocity, gfx::Color4f color)
	{
		// physics
		ConstructSphereBody(ball, sphere, velocity, ball_density, ball_linear_damping, ball_angular_damping);

		// graphics
		gfx::Transformation local_transformation(sphere.center, gfx::Transformation::Matrix33::Identity());
		gfx::ObjectHandle model = gfx::BallHandle::CreateHandle(local_transformation, sphere.radius, color);
		ball.SetModel(model);
	}

	void ConstructObserver(Entity & observer, Vector3 const & position)
	{
		// physics
		if (! observer_use_touch)
		{
			if (observer_physics)
			{
				ConstructSphereBody(observer, geom::rel::Sphere3(position, observer_radius), Vector3::Zero(), observer_density, observer_linear_damping, observer_angular_damping);
			}
			else
			{
				physics::Mass m;
				dMassSetSphere(& m, observer_density, observer_radius);
				ConstructBody(observer, position, Vector3::Zero(), m, observer_linear_damping, observer_angular_damping);
			}
		}

		// controller
		auto controller = [&] () -> Controller *
		{
			if (! observer_use_touch)
			{
				if (SDL_SetRelativeMouseMode(SDL_TRUE) == 0)
				{
					return new MouseObserverController(observer);
				}
				else
				{
					// Linux requires libxi-dev to be installed for this to succeed.
					DEBUG_MESSAGE("Failed to set relative mouse mode.");
				}
			}

			return new TouchObserverController(observer, position);
		} ();
		
		observer.SetController(controller);

#if defined(OBSERVER_LIGHT)
		// register light with the renderer
		gfx::Light * light = new gfx::Light(observer_light_color);
		_light_uid = AddModelWithTransform(* light);
#endif
	}

	void ConstructCamera(Entity & camera, Vector3 const & position, EntityHandle subject_handle)
	{
		// physics
		ConstructSphereBody(camera, geom::rel::Sphere3(position, camera_radius), Vector3::Zero(), camera_density, camera_linear_damping, camera_angular_damping);

		// controller
		camera.SetController(new CameraController(camera, subject_handle));
	}

	void AddThruster(VehicleController & controller, Thruster * thruster)
	{
		controller.AddThruster(VehicleController::ThrusterPtr(thruster));
	}

	void AddHoverThruster(VehicleController & controller, Vector3 const & position, Scalar distance)
	{
		auto & entity = controller.GetEntity();
		AddThruster(controller, new HoverThruster(entity, position, distance));
	}

	void AddRoverThruster(VehicleController & controller, Ray3 const & ray, SDL_Scancode key, bool graphical, bool invert = false)
	{
		auto & entity = controller.GetEntity();
		auto activation_callback = [key, invert] ()
		{
			return (app::IsKeyDown(key) != invert) ? 1.f : 0.f;
		};
		AddThruster(controller, new RoverThruster(entity, ray, activation_callback, graphical));
	}

	void AddVernierThruster(VehicleController & controller, Ray3 const & ray)
	{
		auto & entity = controller.GetEntity();
		AddThruster(controller, new VernierThruster(entity, ray));
	}

	void ConstructRover(Entity & entity, geom::rel::Sphere3 const & sphere)
	{
		ConstructBall(entity, sphere, Vector3::Zero(), gfx::Color4f::White());

		auto& controller = ref(new VehicleController(entity));
		entity.SetController(& controller);

		AddRoverThruster(controller, Ray3(Vector3(.5, -.8f, .5), Vector3(0, 15, 0)), SDL_SCANCODE_H, true);
		AddRoverThruster(controller, Ray3(Vector3(.5, -.8f, -.5), Vector3(0, 15, 0)), SDL_SCANCODE_H, true);
		AddRoverThruster(controller, Ray3(Vector3(-.5, -.8f, .5), Vector3(0, 15, 0)), SDL_SCANCODE_H, true);
		AddRoverThruster(controller, Ray3(Vector3(-.5, -.8f, -.5), Vector3(0, 15, 0)), SDL_SCANCODE_H, true);
	}
	
	void ConstructShip(Entity & entity, Vector3 const & position)
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
		Engine & engine = entity.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto velocity = Vector3::Zero();
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

		if (enable_beam)
		{
			gfx::ObjectHandle beam_handle = gfx::LightHandle::CreateHandle(gfx::Transformation(), gfx::Color4f::Red() * 100000.f, gfx::LightType::beam);
			gfx::Daemon::Call([beam_handle, model_handle] (gfx::Engine & engine) {
				engine.OnSetParent(beam_handle.GetUid(), model_handle.GetUid());
			});
		}

		// controller
		auto & controller = ref(new VehicleController(entity));
		entity.SetController(& controller);

		// add a single thruster
		auto add_thruster = [&] (Ray3 const & ray, SDL_Scancode key, bool invert = false)
		{
			if (key == SDL_SCANCODE_UNKNOWN)
			{
				AddVernierThruster(controller, ray);
			}
			else
			{
				AddRoverThruster(controller, ray, key, true, invert);
			}
		};
		
		// add two complimentary thrusters
		auto add_thrusters = [&] (Ray3 ray, SDL_Scancode first_key, int axis, SDL_Scancode second_key, bool invert = false)
		{
			add_thruster(ray, first_key, invert);
			ray.position[axis] *= -1.f;
			ray.direction[axis] *= -1.f;
			add_thruster(ray, second_key, invert);
		};
		
		auto forward = Vector3(0, 0, ship_forward_thrust);

		add_thrusters(Ray3(Vector3(0.f, 0.f, 1.f), geom::Resized(Vector3(0.f, 1.f, - ship_upward_thrust_gradient), ship_upward_thrust)), SDL_SCANCODE_UNKNOWN, 2, SDL_SCANCODE_UNKNOWN);
		add_thrusters(Ray3(Vector3(1., 0.f, 0), geom::Resized(Vector3(- ship_upward_thrust_gradient, 1.f, 0.f), ship_upward_thrust)), SDL_SCANCODE_UNKNOWN, 0, SDL_SCANCODE_UNKNOWN);
		add_thrusters(Ray3(Vector3(.25f, 0.f, -.525f), forward * .5f), SDL_SCANCODE_RIGHT, 0, SDL_SCANCODE_LEFT, true);
		
		AddHoverThruster(controller, Vector3(0.f, -.25f, 0.f), -.1f);
		AddHoverThruster(controller, Vector3(0.f, .25f, 0.f), .1f);
	}

	void ConstructUfo(Entity & entity, Vector3 const & position)
	{
		// resources
		auto & resource_manager = crag::core::ResourceManager::Get();
		resource_manager.Register<physics::Mesh>("UfoPhysicsMesh", [] ()
		{
			return GenerateUfoMesh<dTriIndex>(ufo_height, ufo_radius);
		});
		resource_manager.Register<gfx::PlainMesh>("UfoPlainMesh", [] ()
		{
			return GenerateUfoMesh<gfx::ElementIndex>(ufo_height, ufo_radius);
		});
		resource_manager.Register<gfx::LitMesh>("UfoLitMesh", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto physics_mesh = resource_manager.GetHandle<physics::Mesh>("UfoPhysicsMesh");
			return GenerateFlatLitMesh(* physics_mesh);
		});
		
		auto lit_mesh_handle = resource_manager.GetHandle<gfx::LitMesh>("UfoLitMesh");
		resource_manager.Register<gfx::LitVboResource>("UfoVbo", [lit_mesh_handle] ()
		{
			return gfx::LitVboResource(* lit_mesh_handle);
		});

		// physics
		Engine & engine = entity.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto rotation = gfx::Rotation(geom::Normalized(position), gfx::Direction::forward);
		Transformation transformation(position, rotation);
		auto velocity = Vector3::Zero();
		auto & body = * new physics::CylinderBody(transformation, & velocity, physics_engine, ufo_radius, ufo_height);
		body.SetLinearDamping(ufo_linear_damping);
		body.SetAngularDamping(ufo_angular_damping);
		entity.SetLocation(& body);

		// graphics
		gfx::Transformation local_transformation(position, gfx::Transformation::Matrix33::Identity());
		gfx::Color4f const & color = gfx::Color4f::White();
		gfx::Vector3 scale(1.f, 1.f, 1.f);
		auto lit_vbo = resource_manager.GetHandle<gfx::LitVboResource>("UfoVbo");
		auto plain_mesh = resource_manager.GetHandle<gfx::PlainMesh>("UfoPlainMesh");
		gfx::ObjectHandle model_handle = gfx::MeshObjectHandle::CreateHandle(local_transformation, color, scale, lit_vbo, plain_mesh);
		entity.SetModel(model_handle);

		// controller
		auto & controller = ref(new UfoController(entity));
		entity.SetController(& controller);

		if (SDL_SetRelativeMouseMode(SDL_TRUE))
		{
			// Linux requires libxi-dev to be installed for this to succeed.
			DEBUG_MESSAGE("Failed to set relative mouse mode.");
		}
	}
}

EntityHandle SpawnBall(Sphere3 const & sphere, Vector3 const & velocity, gfx::Color4f color)
{
	ASSERT(color.a = 1);
	
	// ball
	auto ball = EntityHandle::CreateHandle();

	ball.Call([sphere, velocity, color] (Entity & entity) {
		ConstructBall(entity, sphere, velocity, color);
	});

	return ball;
}

EntityHandle SpawnBox(Vector3 const & position, Vector3 const & velocity, Vector3 const & size, gfx::Color4f color)
{
	ASSERT(color.a = 1);
	
	// box
	auto box = EntityHandle::CreateHandle();

	box.Call([position, velocity, size, color] (Entity & entity) {
		ConstructBox(entity, position, velocity, size, color);
	});

	return box;
}

EntityHandle SpawnObserver(const Vector3 & position)
{
	auto observer = EntityHandle::CreateHandle();

	observer.Call([position] (Entity & entity) {
		ConstructObserver(entity, position);
	});

	return observer;
}

EntityHandle SpawnCamera(Vector3 const & position, EntityHandle subject)
{
	auto camera = EntityHandle::CreateHandle();

	camera.Call([position, subject] (Entity & entity) {
		ConstructCamera(entity, position, subject);
	});

	return camera;
}

EntityHandle SpawnPlanet(const Sphere3 & sphere, int random_seed, int num_craters)
{
	auto handle = EntityHandle::CreateHandle();

	handle.Call([sphere, random_seed, num_craters] (Entity & entity) {
		auto & engine = entity.GetEngine();

		// controller
		auto& controller = ref(new PlanetController(entity, sphere, random_seed, num_craters));
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
EntityHandle SpawnStar(geom::abs::Sphere3 const & volume, gfx::Color4f const & color)
{
	auto sun = EntityHandle::CreateHandle();

	sun.Call([volume, color] (Entity & entity) {
		// physics
		Transformation transformation(geom::Cast<Scalar>(volume.center));
		auto location = new physics::PassiveLocation(transformation);
		entity.SetLocation(location);

		// graphics
		auto light = gfx::LightHandle::CreateHandle(transformation, color, gfx::LightType::shadow);
		entity.SetModel(light);
	});

	return sun;
}

EntityHandle SpawnRover(Vector3 const & position)
{
	auto vehicle = EntityHandle::CreateHandle();

	geom::rel::Sphere3 sphere;
	sphere.center = geom::Cast<float>(position);
	sphere.radius = 1.;

	vehicle.Call([sphere] (Entity & entity) {
		ConstructRover(entity, sphere);
	});

	return vehicle;
}

EntityHandle SpawnShip(Vector3 const & position)
{
	auto ship = EntityHandle::CreateHandle();

	ship.Call([position] (Entity & entity) {
		ConstructShip(entity, position);
	});

	return ship;
}

EntityHandle SpawnUfo(Vector3 const & position)
{
	auto ship = EntityHandle::CreateHandle();

	ship.Call([position] (Entity & entity) {
		ConstructUfo(entity, position);
	});

	return ship;
}
