//
//  SpawnPlayer.cpp
//  crag
//
//  Created by John McFarlane on 2014-04-13.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "SpawnPlayer.h"

#include "entity/sim/HoverThruster.h"
#include "entity/sim/MouseObserverController.h"
#include "entity/sim/RoverThruster.h"
#include "entity/sim/UfoController.h"
#include "entity/sim/TouchObserverController.h"
#include "entity/sim/VehicleController.h"
#include "entity/sim/VernierThruster.h"

#include "sim/Engine.h"
#include "sim/Entity.h"
#include "sim/EntityFunctions.h"

#include "physics/defs.h"
#include "physics/CylinderBody.h"
#include "physics/Engine.h"
#include "physics/GhostBody.h"
#include "physics/MeshBody.h"
#include "physics/SphereBody.h"

#include "form/Mesh.h"

#include "gfx/axes.h"
#include "gfx/Engine.h"
#include "gfx/IndexedVboResource.h"
#include "gfx/LitVertex.h"
#include "gfx/PlainVertex.h"
#include "gfx/object/Ball.h"
#include "gfx/object/SearchLight.h"
#include "gfx/object/MeshObject.h"

#include "geom/origin.h"

#include "core/app.h"
#include "core/ConfigEntry.h"
#include "core/ResourceManager.h"

using namespace std;
using namespace sim;

#if defined(CRAG_USE_MOUSE)
CONFIG_DEFINE (observer_use_touch, bool, false);
#elif defined(CRAG_USE_TOUCH)
CONFIG_DEFINE (observer_use_touch, bool, true);
#endif
CONFIG_DEFINE (observer_physics, bool, false);

namespace gfx
{
	DECLARE_CLASS_HANDLE(Ball); // gfx::BallHandle
	DECLARE_CLASS_HANDLE(Light); // gfx::LightHandle
	DECLARE_CLASS_HANDLE(SearchLight); // gfx::SearchLightHandle
}

namespace
{
	////////////////////////////////////////////////////////////////////////////////
	// Config values

	CONFIG_DEFINE (observer_radius, float, .5);
	CONFIG_DEFINE (observer_density, float, 1);
	
	CONFIG_DEFINE (observer_linear_damping, physics::Scalar, 0.025f);
	CONFIG_DEFINE (observer_angular_damping, physics::Scalar, 0.05f);

	CONFIG_DEFINE (ship_linear_damping, physics::Scalar, 0.05f);
	CONFIG_DEFINE (ship_angular_damping, physics::Scalar, 0.15f);
	CONFIG_DEFINE (ship_upward_thrust, physics::Scalar, 0.25f);
	CONFIG_DEFINE (ship_upward_thrust_gradient, physics::Scalar, 0.75f);
	CONFIG_DEFINE (ship_forward_thrust, physics::Scalar, 10.0f);

	CONFIG_DEFINE (ufo_color1, gfx::Color4f, gfx::Color4f::Green());
	CONFIG_DEFINE (ufo_color2, gfx::Color4f, gfx::Color4f::Red());
	CONFIG_DEFINE (ufo_color3, gfx::Color4f, gfx::Color4f::Green());

	CONFIG_DEFINE (saucer_height, physics::Scalar, .6f);
	CONFIG_DEFINE (saucer_radius, physics::Scalar, 1.f);
	CONFIG_DEFINE (saucer_cylinder_height, physics::Scalar, .01f);
	CONFIG_DEFINE (saucer_ball_radius, physics::Scalar, .45f);
	CONFIG_DEFINE (saucer_ball_density, float, 1);
	CONFIG_DEFINE (saucer_ball_linear_damping, float, 0.005f);
	CONFIG_DEFINE (saucer_ball_angular_damping, float, 0.005f);
	CONFIG_DEFINE (saucer_thrust, float, 16.f);
	CONFIG_DEFINE (saucer_linear_damping, physics::Scalar, 0.01f);
	CONFIG_DEFINE (saucer_angular_damping, physics::Scalar, 0.05f);
	CONFIG_DEFINE (saucer_num_sectors, int, 24);
	CONFIG_DEFINE (saucer_num_rings, int, 5);
	CONFIG_DEFINE (saucer_flat_shade_cos, bool, false);
	CONFIG_DEFINE (saucer_flat_shade_ball, bool, true);
	CONFIG_DEFINE (saucer_search_light_enable, bool, true);
	CONFIG_DEFINE (saucer_search_light_angle, Scalar, .15f);

	CONFIG_DEFINE (thargoid_height, physics::Scalar, .3f);
	CONFIG_DEFINE (thargoid_radius, physics::Scalar, 1.f);
	CONFIG_DEFINE (thargoid_inner_radius_ratio, physics::Scalar, .5f);
	CONFIG_DEFINE (thargoid_thrust, float, 9.f);

	////////////////////////////////////////////////////////////////////////////////
	// mesh generation
	//
	// TODO: A new home for shared concept of mesh and its helpers outside of gfx
	
	bool operator==(gfx::PlainVertex const & lhs, gfx::PlainVertex const & rhs)
	{
		return lhs.pos == rhs.pos;
	}
	
	template <typename VertexType, typename IndexType>
	void AddMeshVertex(gfx::Mesh<VertexType, IndexType> & mesh, VertexType vertex)
	{
		auto & vertices = mesh.GetVertices();
		auto num_vertices = vertices.size();
		
		auto index = 0u;
		for (;;)
		{
			if (index == num_vertices)
			{
				vertices.push_back(vertex);
				break;
			}
			
			if (vertices[index] == vertex)
			{
				break;
			}
			
			++ index;
		}
		
		mesh.GetIndices().push_back(index);
	}
	
	// generates and transforms mesh given a transformation function
	template <typename DestinationMeshType, typename SourceMeshType, typename Function>
	DestinationMeshType TransformMesh(SourceMeshType const & source, Function function)
	{
		DestinationMeshType destination;

		{
			auto const & source_vertices = source.GetVertices();
			auto & destination_vertices = destination.GetVertices();
			destination_vertices.reserve(source_vertices.size());

			auto const & source_indices = source.GetIndices();
			auto & destination_indices = destination.GetIndices();
			destination_indices.reserve(source_indices.size());

			for (auto source_index : source_indices)
			{
				ASSERT(source_index < source_vertices.size());
				auto const & source_vertex = source_vertices[source_index];
				auto destination_vertex = function(source_vertex);
				
				AddMeshVertex(destination, destination_vertex);
			}

			ASSERT(destination_vertices.size() <= source_vertices.size());
			ASSERT(destination_vertices.size() <= destination_vertices.capacity());
			ASSERT(destination_indices.size() == source_indices.size());
			ASSERT(destination_indices.size() == destination_indices.capacity());
			
#if ! defined(NDEBUG)
			if (destination_vertices.size() <= source_vertices.size())
			{
				DEBUG_MESSAGE("%d bytes saved", (source_vertices.size() - destination_vertices.size()) * sizeof(typename DestinationMeshType::value_type));
			}
#endif
		}

		return destination;
	}
	
	// generates and transforms mesh given a transformation function
	template <typename DestinationIndexType, typename SourceMeshType>
	gfx::Mesh<gfx::PlainVertex, DestinationIndexType> LitToPlainMesh(SourceMeshType const & source)
	{
		using SourceVertexType = typename SourceMeshType::value_type;
		using DestinationMeshType = gfx::Mesh<gfx::PlainVertex, DestinationIndexType>;

		return TransformMesh<DestinationMeshType>(source, [] (SourceVertexType const & source_vertex)
		{
			return gfx::PlainVertex { source_vertex.pos };
		});
	}

	// given a Mesh comprising unique vertex entries,
	// generates a GPU-friendly mesh with flat shading
	// TODO: Write a more general-purpose flat-faced mesh converted which scans for dupes as it generates new verts
	gfx::LitMesh GenerateFlatLitMesh(gfx::LitMesh const & source_mesh)
	{
		auto & source_vertices = source_mesh.GetVertices();
		auto & source_indices = source_mesh.GetIndices();
		auto num_source_indices = source_indices.size();

		gfx::LitMesh destination_mesh;

		auto & destination_vertices = destination_mesh.GetVertices();
		auto & destination_indices = destination_mesh.GetIndices();
		
		destination_vertices.reserve(num_source_indices);
		destination_indices.reserve(num_source_indices);

		for (auto end = std::end(source_mesh), source_iterator = std::begin(source_mesh); source_iterator != end; source_iterator += 3)
		{
			gfx::Triangle3 source_triangle;
			for (int i = 0; i != 3; ++ i)
			{
				source_triangle.points[i] = source_iterator[i].pos;
			}

			auto source_plane = geom::MakePlane(source_triangle);
			
			for (int i = 0; i != 3; ++ i)
			{
				destination_indices.push_back(destination_vertices.size());
				
				auto const & source_vertex = source_iterator[i];
				destination_vertices.push_back(gfx::LitVertex(
				{ 
					source_vertex.pos,
					geom::Normalized(source_plane.normal),
					source_vertex.color
				}));
			}
		}
		
		CRAG_VERIFY_EQUAL(source_vertices.size(), source_vertices.capacity());
		CRAG_VERIFY_EQUAL(source_indices.size(), source_indices.capacity());

		CRAG_VERIFY(destination_mesh);
		return destination_mesh;
	}
	
	gfx::LitMesh GenerateShipMesh(gfx::Color4b const & color = gfx::Color4b::White())
	{
		// ship mesh
		gfx::LitMesh mesh;
		
		// add vertices
		auto & vertices = mesh.GetVertices();
		vertices.reserve(5);
		vertices.push_back(gfx::LitVertex { Vector3(0.f, 0.f, 1.f), Vector3(0.f, 0.f, 1.f), color });
		vertices.push_back(gfx::LitVertex { Vector3(-1.f, 0.f, -1.f), Vector3(-1.f, 0.f, 0.f), color });
		vertices.push_back(gfx::LitVertex { Vector3(1.f, 0.f, -1.f), Vector3(1.f, 0.f, 0.f), color });
		vertices.push_back(gfx::LitVertex { Vector3(0.f, -.25f, -1.f), Vector3(0.f, -1.f, 0.f), color });
		vertices.push_back(gfx::LitVertex { Vector3(0.f, .25f, -1.f), Vector3(0.f, 1.f, 0.f), color });
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
	
	// all the work of making half a UFO
	template <
		typename VertexType,
		typename IndexType, 
		typename VertexFn>
	void GenerateUfoMeshSide(gfx::Mesh<VertexType, IndexType> & mesh, VertexFn fn, int num_sectors, int num_rings, int next_sector_offset)
	{
		auto num_vertices = num_sectors * num_rings + 1;
		
		auto num_faces_per_sector = 2 * num_rings - 1;
		auto num_faces = num_sectors * num_faces_per_sector;
		auto num_indices = num_faces * 3;

		// vertices
		auto & vertices = mesh.GetVertices();
		auto vertices_base_index = int(vertices.size());
		auto sector_vertices_index = vertices_base_index + 1;
		int new_vertices_capacity = vertices_base_index + num_vertices;
		vertices.reserve(new_vertices_capacity);

		vertices.push_back(fn(0, 0));
		
		for (auto sector = 0; sector != num_sectors; ++ sector)
		{
			auto angle = float(PI * 2. * sector) / num_sectors;
			
			for (auto ring = 1; ring <= num_rings; ++ ring)
			{
				auto x = float(ring) / num_rings;
				vertices.push_back(fn(angle, x));
			}
		}
		
		CRAG_VERIFY_EQUAL(int(vertices.capacity()), new_vertices_capacity);
		CRAG_VERIFY_EQUAL(int(vertices.size()), new_vertices_capacity);

		// indices
		auto & indices = mesh.GetIndices();
		auto indices_base_index = int(indices.size());
		auto new_indices_capacity = indices_base_index + num_indices;
		indices.reserve(new_indices_capacity);
		
		auto add_face = [& indices] (int a, int b, int c)
		{
			indices.push_back(a);
			indices.push_back(b);
			indices.push_back(c);
		};

		for (auto sector = 0; sector != num_sectors; ++ sector)
		{
			bool odd_sector = (! (num_sectors & 1)) && (sector & 1);
			auto next_sector = (sector + next_sector_offset) % num_sectors;
			
			auto sector_vertex_index = sector_vertices_index + sector * num_rings;
			auto next_sector_vertex_index = sector_vertices_index + next_sector * num_rings;
			
			add_face(vertices_base_index, sector_vertex_index, next_sector_vertex_index);
			
			for (auto ring = 1; ring != num_rings; ++ ring)
			{
				int quad[4] =
				{
					sector_vertex_index + ring - 1,
					next_sector_vertex_index + ring - 1,
					sector_vertex_index + ring,
					next_sector_vertex_index + ring
				};
				
				if (odd_sector)
				{
					indices.push_back(quad[2]);
					indices.push_back(quad[1]);
					indices.push_back(quad[0]);
					indices.push_back(quad[1]);
					indices.push_back(quad[2]);
					indices.push_back(quad[3]);
				}
				else
				{
					indices.push_back(quad[3]);
					indices.push_back(quad[1]);
					indices.push_back(quad[0]);
					indices.push_back(quad[0]);
					indices.push_back(quad[2]);
					indices.push_back(quad[3]);
				}
			}
		}

		CRAG_VERIFY_EQUAL(int(indices.capacity()), new_indices_capacity);
		CRAG_VERIFY_EQUAL(int(indices.size()), new_indices_capacity);
	}
	
	template <typename UpFn, typename DownFn>
	gfx::LitMesh GenerateUfoMesh(UpFn up_fn, DownFn down_fn, int num_sectors, int upper_num_rings, int lower_num_rings)
	{
		gfx::LitMesh mesh;
		
		GenerateUfoMeshSide(mesh, [up_fn] (Scalar angle, Scalar x) {
			auto v = up_fn(angle, x);
			v.color = ufo_color1;
			return v;
		}, num_sectors, upper_num_rings, 1);
		
		GenerateUfoMeshSide(mesh, [down_fn] (Scalar angle, Scalar x) {
			auto v = down_fn(angle, x);
			v.color = ufo_color2;
			return v;
		}, num_sectors, lower_num_rings, num_sectors - 1);
		
		// return result
		CRAG_VERIFY(mesh);
		return mesh;
	}
	
	// generate UFO mesh with horizontal symetry
	template <typename Fn>
	gfx::LitMesh GenerateSymetricalUfoMesh(Fn up_fn, int num_sectors, int num_rings)
	{
		auto down_fn = [up_fn] (Scalar angle, Scalar x)
		{
			auto v = up_fn(angle, x);
			v.pos.z *= -1.f;
			v.norm.z *= -1.f;
			return v;
		};
		
		return GenerateUfoMesh(up_fn, down_fn, num_sectors, num_rings, num_rings);
	}

	// plain Thargoid mesh
	gfx::LitMesh GenerateThargoidMesh(Scalar height, Scalar radius)
	{
		auto half_height = height * .5f;
		gfx::LitMesh mesh;
		
		auto up_fn = [half_height, radius] (Scalar angle, Scalar d)
		{
			Scalar x, y;
			if (d < .25f)
			{
				ASSERT(d == 0.f);
				x = 0;
				y = half_height;
			}
			else if (d < .75f)
			{
				ASSERT(d == .5f);
				x = thargoid_inner_radius_ratio;
				y = half_height;
			}
			else
			{
				ASSERT(d == 1.f);
				x = 1.f;
				y = - half_height;
			}

			auto radial = Vector2(std::sin(angle), std::cos(angle));
			auto position = Vector3(radial.x * x, radial.y * x, y);

			auto slope_angle = std::atan(half_height / (radius * (1.f - thargoid_inner_radius_ratio)));
			auto normal_angle = slope_angle * .5f;
			auto normal_slice = Vector2(std::sin(normal_angle), std::cos(normal_angle));
			auto normal = Vector3(normal_slice.x * radial.x, normal_slice.x * radial.y, normal_slice.y);
			CRAG_VERIFY_NEARLY_EQUAL(geom::Length(normal), 1.f, .001f);

			return gfx::LitVertex
			{
				position,
				normal, 
				gfx::Color4b::White()
			};
		};
		GenerateUfoMeshSide(mesh, up_fn, 8, 2, 1);

		auto down_fn = [half_height, radius] (Scalar angle, Scalar d)
		{
			ASSERT(d == 0.f || d == 1.f);
			Scalar x = d * radius, y = - half_height;

			auto radial = Vector2(std::sin(angle), std::cos(angle));
			auto position = Vector3(radial.x * x, radial.y * x, y);

			return gfx::LitVertex
			{
				position,
				Vector3(0.f, 0.f, -1.f), 
				gfx::Color4b::White()
			};
		};
		GenerateUfoMeshSide(mesh, down_fn, 8, 1, 7);

		return mesh;
	}
	
	// Cos-shaped Saucer mesh
	gfx::LitMesh GenerateCosSaucerMesh(Scalar height, Scalar radius)
	{
		auto half_height = height;
		auto up = [half_height, radius] (Scalar angle, Scalar d)
		{
			auto radial = Vector3(std::sin(angle), std::cos(angle), 1.f);
			auto slope_angle = d * Scalar(PI);
			
			auto x = radius * d;
			auto y = half_height * (.5f * std::cos(slope_angle) + .5f);
			
			auto dx = radius;
			auto dy = std::sin(slope_angle);
			
			auto position = radial * Vector3(x, x, y);
			auto normal = geom::Normalized(radial * Vector3(dy, dy, dx));

			return gfx::LitVertex
			{
				position, 
				normal, 
				gfx::Color4b::White()
			};
		};
		
		return GenerateSymetricalUfoMesh(up, saucer_num_sectors, saucer_num_rings);
	}

	// dish-shaped Saucer mesh (is added to sphere to make ball saucer)
	gfx::LitMesh GenerateBallSaucerMesh(Scalar height, Scalar radius)
	{
		auto half_height = height * .5f;

		auto up_fn = [half_height, radius] (Scalar angle, Scalar d)
		{
			ASSERT(d == 0.f || d == 1.f);
			
			Scalar x, y;
			if (d < .5f)
			{
				ASSERT(d == 0.f);
				x = 0;
				y = half_height;
			}
			else
			{
				ASSERT(d == 1.f);
				x = 1.f;
				y = 0.f;
			}

			auto radial = Vector2(std::sin(angle), std::cos(angle));
			auto position = Vector3(radial.x * x, radial.y * x, y);

			auto slope_angle = std::atan(half_height / radius);
			auto normal_angle = slope_angle * .5f;
			auto normal_slice = Vector2(std::sin(normal_angle), std::cos(normal_angle));
			auto normal = Vector3(normal_slice.x * radial.x, normal_slice.x * radial.y, normal_slice.y);
			CRAG_VERIFY_NEARLY_EQUAL(geom::Length(normal), 1.f, .001f);

			return gfx::LitVertex
			{
				position,
				normal, 
				gfx::Color4b::White()
			};
		};

		return GenerateSymetricalUfoMesh(up_fn, saucer_num_sectors, 1);
	}
	
	////////////////////////////////////////////////////////////////////////////////
	// entity composition
	
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
		ConstructSphereBody(ball, sphere, velocity, saucer_ball_density, saucer_ball_linear_damping, saucer_ball_angular_damping);

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

	void ConstructRover(Entity & entity, geom::rel::Sphere3 const & sphere, Scalar thrust)
	{
		ConstructBall(entity, sphere, Vector3::Zero(), gfx::Color4f::White());

		auto& controller = ref(new VehicleController(entity));
		entity.SetController(& controller);

		AddRoverThruster(controller, Ray3(Vector3(.5, -.8f, .5), Vector3(0, thrust, 0)), SDL_SCANCODE_H, true);
		AddRoverThruster(controller, Ray3(Vector3(.5, -.8f, -.5), Vector3(0, thrust, 0)), SDL_SCANCODE_H, true);
		AddRoverThruster(controller, Ray3(Vector3(-.5, -.8f, .5), Vector3(0, thrust, 0)), SDL_SCANCODE_H, true);
		AddRoverThruster(controller, Ray3(Vector3(-.5, -.8f, -.5), Vector3(0, thrust, 0)), SDL_SCANCODE_H, true);
	}
	
	void ConstructShip(Entity & entity, Vector3 const & position)
	{
		Engine & engine = entity.GetEngine();
		auto & resource_manager = crag::core::ResourceManager::Get();

		// physics
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto velocity = Vector3::Zero();
		auto physics_mesh = resource_manager.GetHandle<physics::Mesh>("ShipPhysicsMesh");
		auto & body = * new physics::MeshBody(position, & velocity, physics_engine, * physics_mesh);
		body.SetLinearDamping(ship_linear_damping);
		body.SetAngularDamping(ship_angular_damping);
		entity.SetLocation(& body);

		// graphics
		gfx::Transformation local_transformation(position, gfx::Transformation::Matrix33::Identity());
		gfx::Vector3 scale(1.f, 1.f, 1.f);
		auto lit_vbo = resource_manager.GetHandle<gfx::LitVboResource>("ShipVbo");
		auto plain_mesh = resource_manager.GetHandle<gfx::PlainMesh>("ShipShadowMesh");
		gfx::ObjectHandle model_handle = gfx::MeshObjectHandle::CreateHandle(local_transformation, gfx::Color4f::White(), scale, lit_vbo, plain_mesh);
		entity.SetModel(model_handle);

		// controller
		auto & controller = ref(new VehicleController(entity));
		entity.SetController(& controller);

		// add a single thruster
		auto add_thruster = [&] (Ray3 const & ray, SDL_Scancode key, bool invert)
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
		auto add_thrusters = [&] (Ray3 ray, SDL_Scancode first_key, int axis, SDL_Scancode second_key, bool invert)
		{
			add_thruster(ray, first_key, invert);
			ray.position[axis] *= -1.f;
			ray.direction[axis] *= -1.f;
			add_thruster(ray, second_key, invert);
		};
		
		auto forward = Vector3(0, 0, ship_forward_thrust);

		add_thrusters(Ray3(Vector3(0.f, 0.f, 1.f), geom::Resized(Vector3(0.f, 1.f, - ship_upward_thrust_gradient), ship_upward_thrust)), SDL_SCANCODE_UNKNOWN, 2, SDL_SCANCODE_UNKNOWN, false);
		add_thrusters(Ray3(Vector3(1., 0.f, 0), geom::Resized(Vector3(-ship_upward_thrust_gradient, 1.f, 0.f), ship_upward_thrust)), SDL_SCANCODE_UNKNOWN, 0, SDL_SCANCODE_UNKNOWN, false);
		add_thrusters(Ray3(Vector3(.25f, 0.f, -.525f), forward * .5f), SDL_SCANCODE_RIGHT, 0, SDL_SCANCODE_LEFT, true);
		
		AddHoverThruster(controller, Vector3(0.f, -.25f, 0.f), -.1f);
		AddHoverThruster(controller, Vector3(0.f, .25f, 0.f), .1f);
	}

	void ConstructUfo(Entity & ufo_entity, Vector3 const & position, crag::core::HashString vbo_name, crag::core::HashString shadow_mesh_name, PlayerType player_type, Scalar thrust, Scalar radius)
	{
		bool is_thargoid = player_type == PlayerType::thargoid;
		
		// misc preparation
		Engine & engine = ufo_entity.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto velocity = Vector3::Zero();
		gfx::Transformation local_transformation(position, gfx::Transformation::Matrix33::Identity());

		// resources
		auto & resource_manager = crag::core::ResourceManager::Get();
		auto vbo = resource_manager.GetHandle<gfx::LitVboResource>(vbo_name);
		auto shadow_mesh = resource_manager.GetHandle<gfx::PlainMesh>(shadow_mesh_name);

		// saucer physics
		auto rotation = gfx::Rotation(geom::Normalized(position), gfx::Direction::forward);
		Transformation transformation(position, rotation);
		auto & body = * new physics::CylinderBody(transformation, & velocity, physics_engine, radius, is_thargoid ? thargoid_height : saucer_cylinder_height);
		body.SetLinearDamping(saucer_linear_damping);
		body.SetAngularDamping(saucer_angular_damping);
		ufo_entity.SetLocation(& body);

		// graphics
		gfx::Vector3 scale(1.f, 1.f, 1.f);
		gfx::ObjectHandle model_handle = gfx::MeshObjectHandle::CreateHandle(local_transformation, gfx::Color4f::White(), scale, vbo, shadow_mesh);
		ufo_entity.SetModel(model_handle);

		////////////////////////////////////////////////////////////////////////////////
		// ball
		
		shared_ptr<Entity> ball_entity;
		gfx::ObjectHandle exception_object;
		
		if (player_type == PlayerType::cos_saucer || player_type == PlayerType::ball_saucer)
		{
			Sphere3 sphere(position, saucer_ball_radius);
			ball_entity = engine.CreateObject<Entity>();

			// physics
			auto & ball_body = * new physics::SphereBody(sphere.center, & velocity, physics_engine, sphere.radius);
			ball_entity->SetLocation(& ball_body);
		
			AttachEntities(ufo_entity, * ball_entity, physics_engine);
			ball_body.SetIsCollidable(body, false);
			body.SetIsCollidable(ball_body, false);

			if (player_type == PlayerType::ball_saucer)
			{
				// graphics
				gfx::ObjectHandle model = gfx::BallHandle::CreateHandle(local_transformation, sphere.radius, ufo_color3);
				ball_entity->SetModel(model);
				
				exception_object = model;
			}
			else
			{
				exception_object = model_handle;
			}
		}
		
		////////////////////////////////////////////////////////////////////////////////
		// saucer
		
		// searchlight
		if (saucer_search_light_enable)
		{
			// distance between saucer mesh and ball (in the case of ball saucer);
			// ensures light is clear of enclosing object that would extinguish it
			float clear_distance = (saucer_height * .5f + saucer_ball_radius) * .5f;
			
			gfx::Transformation search_light_transformation(Vector3(0.f, 0.f, - clear_distance));
			gfx::ObjectHandle light_handle = gfx::SearchLightHandle::CreateHandle(
				search_light_transformation, 
				gfx::Color4f(.25f, .5f, 1.f) * 20.f, 
				Vector2(std::sin(saucer_search_light_angle), std::cos(saucer_search_light_angle)),
				exception_object);
			gfx::Daemon::Call([light_handle, model_handle] (gfx::Engine & engine) {
				engine.OnSetParent(light_handle, model_handle);
			});
		}

		// controller
		auto & controller = ref(new UfoController(ufo_entity, ball_entity, thrust));
		ufo_entity.SetController(& controller);

		if (SDL_SetRelativeMouseMode(SDL_TRUE))
		{
			// Linux requires libxi-dev to be installed for this to succeed.
			DEBUG_MESSAGE("Failed to set relative mouse mode.");
		}
	}
	
	void AddUfoResources()
	{
		// TODO: When physcis and gfx index are same size, return one resource from the other
		auto & resource_manager = crag::core::ResourceManager::Get();

		// ship
		resource_manager.Register<gfx::LitMesh>("ShipLitMesh", [] ()
		{
			return GenerateShipMesh();
		});
		resource_manager.Register<physics::Mesh>("ShipPhysicsMesh", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh = resource_manager.GetHandle<gfx::LitMesh>("ShipLitMesh");
			return LitToPlainMesh<physics::ElementIndex>(* lit_mesh);
		});
		resource_manager.Register<gfx::PlainMesh>("ShipShadowMesh", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh = resource_manager.GetHandle<gfx::LitMesh>("ShipLitMesh");
			return LitToPlainMesh<gfx::ElementIndex>(* lit_mesh);
		});
		resource_manager.Register<gfx::LitMesh>("ShipFlatLitMesh", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh = resource_manager.GetHandle<gfx::LitMesh>("ShipLitMesh");
			return GenerateFlatLitMesh(* lit_mesh);
		});
		
		resource_manager.Register<gfx::LitVboResource>("ShipVbo", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh_handle = resource_manager.GetHandle<gfx::LitMesh>("ShipFlatLitMesh");
			return gfx::LitVboResource(* lit_mesh_handle);
		});

		// cos saucer
		resource_manager.Register<gfx::LitMesh>("CosSaucerLitMesh", [] ()
		{
			return GenerateCosSaucerMesh(saucer_ball_radius, saucer_radius);
		});
		resource_manager.Register<physics::Mesh>("CosSaucerPhysicsMesh", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh = resource_manager.GetHandle<gfx::LitMesh>("CosSaucerLitMesh");
			return LitToPlainMesh<physics::ElementIndex>(* lit_mesh);
		});
		resource_manager.Register<gfx::PlainMesh>("CosSaucerShadowMesh", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh = resource_manager.GetHandle<gfx::LitMesh>("CosSaucerLitMesh");
			return LitToPlainMesh<gfx::ElementIndex>(* lit_mesh);
		});
		resource_manager.Register<gfx::LitMesh>("CosSaucerFlatLitMesh", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh = resource_manager.GetHandle<gfx::LitMesh>("CosSaucerLitMesh");
			return GenerateFlatLitMesh(* lit_mesh);
		});
		
		resource_manager.Register<gfx::LitVboResource>("CosSaucerVbo", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh_handle = resource_manager.GetHandle<gfx::LitMesh>("CosSaucerLitMesh");
			return gfx::LitVboResource(* lit_mesh_handle);
		});
		resource_manager.Register<gfx::LitVboResource>("CosSaucerFlatLitVbo", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh_handle = resource_manager.GetHandle<gfx::LitMesh>("CosSaucerFlatLitMesh");
			return gfx::LitVboResource(* lit_mesh_handle);
		});

		// ball saucer
		resource_manager.Register<gfx::LitMesh>("BallSaucerLitMesh", [] ()
		{
			return GenerateBallSaucerMesh(saucer_height, saucer_radius);
		});
		resource_manager.Register<gfx::LitMesh>("BallSaucerFlatLitMesh", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh_handle = resource_manager.GetHandle<gfx::LitMesh>("BallSaucerLitMesh");
			return GenerateFlatLitMesh(* lit_mesh_handle);
		});
		resource_manager.Register<physics::Mesh>("BallSaucerPhysicsMesh", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh = resource_manager.GetHandle<gfx::LitMesh>("BallSaucerLitMesh");
			return LitToPlainMesh<physics::ElementIndex>(* lit_mesh);
		});
		resource_manager.Register<gfx::PlainMesh>("BallSaucerShadowMesh", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh = resource_manager.GetHandle<gfx::LitMesh>("BallSaucerLitMesh");
			return LitToPlainMesh<gfx::ElementIndex>(* lit_mesh);
		});
		
		resource_manager.Register<gfx::LitVboResource>("BallSaucerVbo", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh_handle = resource_manager.GetHandle<gfx::LitMesh>("BallSaucerLitMesh");
			return gfx::LitVboResource(* lit_mesh_handle);
		});
		resource_manager.Register<gfx::LitVboResource>("BallSaucerFlatLitVbo", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh_handle = resource_manager.GetHandle<gfx::LitMesh>("BallSaucerFlatLitMesh");
			return gfx::LitVboResource(* lit_mesh_handle);
		});

		// thargoid
		resource_manager.Register<gfx::LitMesh>("ThargoidLitMesh", [] ()
		{
			return GenerateThargoidMesh(thargoid_height, thargoid_radius);
		});
		resource_manager.Register<gfx::LitMesh>("ThargoidFlatLitMesh", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh_handle = resource_manager.GetHandle<gfx::LitMesh>("ThargoidLitMesh");
			return GenerateFlatLitMesh(* lit_mesh_handle);
		});
		resource_manager.Register<physics::Mesh>("ThargoidPhysicsMesh", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh = resource_manager.GetHandle<gfx::LitMesh>("ThargoidLitMesh");
			return LitToPlainMesh<physics::ElementIndex>(* lit_mesh);
		});
		resource_manager.Register<gfx::PlainMesh>("ThargoidShadowMesh", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh = resource_manager.GetHandle<gfx::LitMesh>("ThargoidLitMesh");
			return LitToPlainMesh<gfx::ElementIndex>(* lit_mesh);
		});
		
		resource_manager.Register<gfx::LitVboResource>("ThargoidVbo", [] ()
		{
			auto & resource_manager = crag::core::ResourceManager::Get();
			auto lit_mesh_handle = resource_manager.GetHandle<gfx::LitMesh>("ThargoidFlatLitMesh");
			return gfx::LitVboResource(* lit_mesh_handle);
		});
	}
}

EntityHandle SpawnRover(Vector3 const & position, Scalar thrust)
{
	auto vehicle = EntityHandle::CreateHandle();

	geom::rel::Sphere3 sphere;
	sphere.center = geom::Cast<float>(position);
	sphere.radius = 1.;

	vehicle.Call([sphere, thrust] (Entity & entity) {
		ConstructRover(entity, sphere, thrust);
	});

	return vehicle;
}

sim::EntityHandle SpawnPlayer(sim::Vector3 const & position, PlayerType player_type)
{
	AddUfoResources();
	
	auto ship = EntityHandle::CreateHandle();

	ship.Call([=] (Entity & entity) {
		switch (player_type)
		{
		case PlayerType::observer:
			ConstructObserver(entity, position);
			break;

		case PlayerType::arrow:
			ConstructShip(entity, position);
			break;

		case PlayerType::thargoid:
			ConstructUfo(entity, position, "ThargoidVbo", "ThargoidShadowMesh", player_type, thargoid_thrust, thargoid_radius);
			break;

		case PlayerType::cos_saucer:
			ConstructUfo(entity, position, saucer_flat_shade_cos ? "CosSaucerFlatLitVbo" : "CosSaucerVbo", "CosSaucerShadowMesh", player_type, saucer_thrust, saucer_radius);
			break;

		case PlayerType::ball_saucer:
			ConstructUfo(entity, position, saucer_flat_shade_ball ? "BallSaucerFlatLitVbo" : "BallSaucerVbo", "BallSaucerShadowMesh", player_type, saucer_thrust, saucer_radius);
			break;
		}
	});

	return ship;
}
