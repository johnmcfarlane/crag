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
#include "SpawnEntityFunctions.h"

#include <entity/sim/KeyboardTransmitter.h>
#include "entity/sim/MouseObserverController.h"
#include <entity/sim/Sensor.h>
#include <entity/sim/Thruster.h>
#include "entity/sim/TouchObserverController.h"
#include "entity/sim/UfoController1.h"
#include "entity/sim/UfoController2.h"
#include "entity/sim/VehicleController.h"

#include "sim/Engine.h"
#include "sim/Entity.h"
#include "sim/EntityFunctions.h"
#include "sim/gravity.h"
#include <sim/Model.h>

#include "physics/defs.h"
#include "physics/CylinderBody.h"
#include "physics/Engine.h"
#include "physics/GhostBody.h"
#include "physics/MeshBody.h"
#include "physics/SphereBody.h"

#include "form/Mesh.h"

#include "gfx/Engine.h"
#include "gfx/IndexedVboResource.h"
#include "gfx/LitVertex.h"
#include "gfx/PlainVertex.h"
#include "gfx/object/Ball.h"
#include "gfx/object/SearchLight.h"
#include "gfx/object/MeshObject.h"

#include "geom/Space.h"
#include <geom/utils.h>

#include "core/app.h"
#include "core/ConfigEntry.h"
#include "core/GlobalResourceManager.h"

using namespace sim;

#if defined(CRAG_USE_MOUSE)
CONFIG_DEFINE(observer_use_touch, false);
#elif defined(CRAG_USE_TOUCH)
CONFIG_DEFINE(observer_use_touch, true);
#endif
CONFIG_DEFINE(grab_input, true);
CONFIG_DEFINE(observer_physics, false);
CONFIG_DEFINE(ufo_controller_type, 2);
CONFIG_DEFINE(saucer_ball_radius, .45f);

namespace gfx
{
	DECLARE_CLASS_HANDLE(Ball); // gfx::BallHandle
	DECLARE_CLASS_HANDLE(Light); // gfx::LightHandle
	DECLARE_CLASS_HANDLE(SearchLight); // gfx::SearchLightHandle
}

namespace
{
	enum class PlayerType
	{
		observer,
		arrow,
		thargoid,
		cos_saucer,
		ball_saucer
	};

	////////////////////////////////////////////////////////////////////////////////
	// Config values

	CONFIG_DEFINE(observer_radius, .5f);
	CONFIG_DEFINE(observer_density, 1.f);
	CONFIG_DEFINE(observer_translation_coefficient_ghost, 1.f);
	CONFIG_DEFINE(observer_translation_coefficient_physical, 2.f);

	CONFIG_DEFINE(ship_angular_damping, .25f);
	CONFIG_DEFINE(ship_upward_thrust_gradient, 1.f);
	CONFIG_DEFINE(ship_forward_thrust, 5.0f);
	CONFIG_DEFINE(ship_vernier_scan_depth, 35.f);
	CONFIG_DEFINE(ship_vernier_force, -1.5f);
	CONFIG_DEFINE(ship_vernier_elevation, .1f);

	CONFIG_DEFINE(ufo_color1, gfx::Color4f::Green());
	CONFIG_DEFINE(ufo_color2, gfx::Color4f::Red());
	CONFIG_DEFINE(ufo_color3, gfx::Color4f::Green());

	CONFIG_DEFINE(saucer_height, .6f);
	CONFIG_DEFINE(saucer_radius, 1.f);
	CONFIG_DEFINE(saucer_cylinder_height, .01f);
	CONFIG_DEFINE(saucer_ball_density, 1.f);
	CONFIG_DEFINE(saucer_thrust, 16.f);
	CONFIG_DEFINE(saucer_num_sectors, 24);
	CONFIG_DEFINE(saucer_num_rings, 5);
	CONFIG_DEFINE(saucer_flat_shade_cos, false);
	CONFIG_DEFINE(saucer_flat_shade_ball, false);
#if defined(CRAG_GL)
	CONFIG_DEFINE(saucer_search_light_enable, true);
#endif
#if defined(CRAG_GLES)
	CONFIG_DEFINE(saucer_search_light_enable, false);
#endif
	CONFIG_DEFINE(saucer_search_light_angle, .15f);

	CONFIG_DEFINE(thargoid_height, .3f);
	CONFIG_DEFINE(thargoid_radius, 1.f);
	CONFIG_DEFINE(thargoid_inner_radius_ratio, .5f);
	CONFIG_DEFINE(thargoid_thrust, 9.f);

#if defined(CRAG_USE_MOUSE)
	CONFIG_DEFINE(player_type, 0);
#endif
#if defined(CRAG_USE_TOUCH)
#if defined(CRAG_GL)
	CONFIG_DEFINE(player_type, 4);
#endif
#if defined(CRAG_GLES)
	CONFIG_DEFINE(player_type, 3);
#endif
#endif

	CONFIG_DEFINE(camera_start_offset, Vector3(-15, 0, 0));

	////////////////////////////////////////////////////////////////////////////////
	// mesh generation
	//
	// TODO: A new home for shared concept of mesh and its helpers outside of gfx
	
	bool operator==(gfx::PlainVertex const & lhs, gfx::PlainVertex const & rhs)
	{
		return lhs.pos == rhs.pos;
	}

	// adds given vertex to the given mesh
	template <typename VertexType, typename IndexType>
	void AddMeshVertex(gfx::Mesh<VertexType, IndexType> & mesh, VertexType vertex)
	{
		auto & vertices = mesh.GetVertices();
		auto begin = std::begin(vertices);
		auto end = std::end(vertices);

		// find an identical existing vertex
		auto match = std::find_if(begin, end, [& vertex] (VertexType const & existing) {
			return existing == vertex;
		});

		// get its vertex (or one past the end if not found)
		auto index = core::get_index(begin, *match);

		// if no match found,
		if (match == end)
		{
			// add new vertex
			vertices.push_back(vertex);
		}

		CRAG_VERIFY_OP(index, >=, 0);
		CRAG_VERIFY_OP(index, <, int(vertices.size()));

		// either way, add index
		auto & indices = mesh.GetIndices();
		indices.push_back(index);
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
				ASSERT(source_index < static_cast<int>(source_vertices.size()));
				auto const & source_vertex = source_vertices[source_index];
				auto destination_vertex = function(source_vertex);
				
				AddMeshVertex(destination, destination_vertex);
			}

			ASSERT(destination_vertices.size() <= source_vertices.size());
			ASSERT(destination_vertices.size() <= destination_vertices.capacity());
			ASSERT(destination_indices.size() == source_indices.size());
			ASSERT(destination_indices.size() == destination_indices.capacity());
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
		auto const & source_vertices = source_mesh.GetVertices();
		auto const & source_indices = source_mesh.GetIndices();
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
				destination_indices.push_back(static_cast<int>(destination_vertices.size()));
				
				auto const & source_vertex = source_iterator[i];
				destination_vertices.push_back(gfx::LitVertex
				{ 
					source_vertex.pos,
					geom::Normalized(source_plane.normal),
					source_vertex.color
				});
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
			CRAG_VERIFY_NEARLY_EQUAL(geom::Magnitude(normal), 1.f, .001f);

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
			CRAG_VERIFY_NEARLY_EQUAL(geom::Magnitude(normal), 1.f, .001f);

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
	
	void ConstructBody(Entity & entity, Transformation const & transformation, Vector3 const & velocity, physics::Mass m)
	{
		Engine & engine = entity.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto body = std::unique_ptr<physics::GhostBody>(
			new physics::GhostBody(transformation, velocity, physics_engine));
		
		// setting the mass of a shapeless body is somewhat nonsensical
		body->SetMass(m);
		entity.SetLocation(std::move(body));
	}

	void ConstructSphereBody(Entity & entity, Transformation const & transformation, Scalar radius, Vector3 const & velocity, float density)
	{
		Engine & engine = entity.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto body = std::unique_ptr<physics::SphereBody>(
			new physics::SphereBody(transformation, & velocity, physics_engine, radius));
		body->SetDensity(density);
		entity.SetLocation(std::move(body));
	}

	void ConstructBall(Entity & ball, Sphere3 sphere, Vector3 const & velocity, gfx::Color4f color)
	{
		// physics
		ConstructSphereBody(ball, sphere.center, sphere.radius, velocity, saucer_ball_density);

		// graphics
		gfx::Transformation local_transformation(sphere.center, gfx::Transformation::Matrix33::Identity());
		gfx::ObjectHandle model_handle = gfx::BallHandle::Create(local_transformation, sphere.radius, color);
		ball.SetModel(Entity::ModelPtr(new Model(model_handle, * ball.GetLocation())));
	}

	void ConstructObserver(Entity & observer, Transformation const & transformation)
	{
		// physics
		if (! observer_use_touch)
		{
			if (observer_physics)
			{
				ConstructSphereBody(observer, transformation, observer_radius, Vector3::Zero(), observer_density);
			}
			else
			{
				physics::Mass m;
				dMassSetSphere(& m, observer_density, observer_radius);
				ConstructBody(observer, transformation, Vector3::Zero(), m);
			}
		}

		// controller
		auto controller = [&] () -> std::unique_ptr<Controller>
		{
			if (! observer_use_touch)
			{
				if (! grab_input || SDL_SetRelativeMouseMode(SDL_TRUE) == 0)
				{
					return std::unique_ptr<Controller>(new MouseObserverController(
						observer,
						observer_physics ? observer_translation_coefficient_physical : observer_translation_coefficient_ghost));
				}
				else
				{
					// Linux requires libxi-dev to be installed for this to succeed.
					DEBUG_MESSAGE("Failed to set relative mouse mode.");
				}
			}

			return std::unique_ptr<Controller>(new TouchObserverController(observer, transformation.GetTranslation()));
		} ();
		
		observer.SetController(std::move(controller));

#if defined(OBSERVER_LIGHT)
		// register light with the renderer
		gfx::Light * light = new gfx::Light(observer_light_color);
		_light_uid = AddModelWithTransform(* light);
#endif
	}

	void MakeSignalPair(
		VehicleController & controller,
		std::unique_ptr<Transmitter> && transmitter,
		std::unique_ptr<Receiver> && receiver)
	{
		transmitter->AddReceiver(* receiver);

		controller.AddTransmitter(std::move(transmitter));
		controller.AddReceiver(std::move(receiver));
	}

	void AddRoverThruster(VehicleController & controller, Ray3 const & ray, SDL_Scancode key, bool graphical, bool invert = false)
	{
		auto & entity = controller.GetEntity();

		auto down_signal = invert ? 0.f : 1.f;
		auto up_signal = invert ? 1.f : 0.f;

		auto keyboard_transmitter = std::unique_ptr<Transmitter>(new KeyboardTransmitter(key, down_signal, up_signal));
		auto thruster = std::unique_ptr<Receiver>(new Thruster(entity, ray, graphical, up_signal));

		MakeSignalPair(controller, std::move(keyboard_transmitter), std::move(thruster));
	}

	void AddVernierThruster(VehicleController & controller, Ray3 ray, Scalar sensor_length, Scalar thrust)
	{
		geom::Normalize(ray);

		auto & entity = controller.GetEntity();

		auto sensor = std::unique_ptr<Transmitter>(new Sensor(entity, ray, sensor_length));
		auto thruster = std::unique_ptr<Receiver>(new Thruster(entity, ray * thrust, false, 0.f));

		MakeSignalPair(controller, std::move(sensor), std::move(thruster));
	}

	void ConstructRover(Entity & entity, Sphere3 const & sphere, Scalar thrust)
	{
		ConstructBall(entity, sphere, Vector3::Zero(), gfx::Color4f::White());

		auto controller = new VehicleController(entity);
		entity.SetController(std::unique_ptr<VehicleController>(controller));

		AddRoverThruster(* controller, Ray3(Vector3(.5f, -.8f, .5f), Vector3(0.f, thrust, 0.f)), SDL_SCANCODE_H, true);
		AddRoverThruster(* controller, Ray3(Vector3(.5f, -.8f, -.5f), Vector3(0.f, thrust, 0.f)), SDL_SCANCODE_H, true);
		AddRoverThruster(* controller, Ray3(Vector3(-.5f, -.8f, .5f), Vector3(0.f, thrust, 0.f)), SDL_SCANCODE_H, true);
		AddRoverThruster(* controller, Ray3(Vector3(-.5f, -.8f, -.5f), Vector3(0.f, thrust, 0.f)), SDL_SCANCODE_H, true);
	}

	void ConstructShip(Entity & entity, Vector3 const & position)
	{
		Engine & engine = entity.GetEngine();

		// physics
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto velocity = Vector3::Zero();
		auto physics_mesh = crag::GlobalResourceManager::GetHandle<physics::Mesh>("ShipPhysicsMesh");
		auto body = std::unique_ptr<physics::MeshBody>(
			new physics::MeshBody(position, & velocity, physics_engine, * physics_mesh));
		body->SetAngularDamping(ship_angular_damping);
		entity.SetLocation(std::move(body));

		// graphics
		gfx::Transformation local_transformation(position, gfx::Transformation::Matrix33::Identity());
		gfx::Vector3 scale(1.f, 1.f, 1.f);
		gfx::ObjectHandle model_handle = gfx::MeshObjectHandle::Create(local_transformation, gfx::Color4f::White(), scale, "ShipVbo", "ShipShadowMesh");
		entity.SetModel(Entity::ModelPtr(new Model(model_handle, * entity.GetLocation())));

		// create controller
		auto controller = std::unique_ptr<VehicleController>(new VehicleController(entity));

		// add two complimentary thrusters
		auto forward = Vector3(0.f, 0.f, ship_forward_thrust);
		AddRoverThruster(* controller, Ray3(Vector3( .25f, 0.f, -.525f), forward * .5f), SDL_SCANCODE_RIGHT, false, true);
		AddRoverThruster(* controller, Ray3(Vector3(-.25f, 0.f, -.525f), forward * .5f), SDL_SCANCODE_LEFT, false, true);

		AddVernierThruster(* controller, Ray3(Vector3( 1.f, ship_vernier_elevation,  0.f), Vector3(  ship_upward_thrust_gradient, - 1.f, 0.f)), ship_vernier_scan_depth, ship_vernier_force);
		AddVernierThruster(* controller, Ray3(Vector3(-1.f, ship_vernier_elevation,  0.f), Vector3(- ship_upward_thrust_gradient, - 1.f, 0.f)), ship_vernier_scan_depth, ship_vernier_force);
		AddVernierThruster(* controller, Ray3(Vector3( 0.f, ship_vernier_elevation,  1.f), Vector3(0.f, - 1.f,   ship_upward_thrust_gradient)), ship_vernier_scan_depth, ship_vernier_force);
		AddVernierThruster(* controller, Ray3(Vector3( 0.f, ship_vernier_elevation, -1.f), Vector3(0.f, - 1.f, - ship_upward_thrust_gradient)), ship_vernier_scan_depth, ship_vernier_force);

		// assign controller
		entity.SetController(std::move(controller));
	}

	void ConstructUfo(Entity & ufo_entity, Transformation const & transformation, crag::core::HashString vbo_name, crag::core::HashString shadow_mesh_name, PlayerType _player_type, Scalar thrust, Scalar radius)
	{
		bool is_thargoid = _player_type == PlayerType::thargoid;
		
		// misc preparation
		Engine & engine = ufo_entity.GetEngine();
		physics::Engine & physics_engine = engine.GetPhysicsEngine();

		auto velocity = Vector3::Zero();

		// create saucer physics
		auto body = std::unique_ptr<physics::CylinderBody>(
			new physics::CylinderBody(transformation, & velocity, physics_engine, radius, is_thargoid ? thargoid_height : saucer_cylinder_height));

		// graphics
		gfx::Vector3 scale(1.f, 1.f, 1.f);
		gfx::ObjectHandle saucer_model_handle = gfx::MeshObjectHandle::Create(transformation, gfx::Color4f::White(), scale, vbo_name, shadow_mesh_name);
		ufo_entity.SetModel(Entity::ModelPtr(new Model(saucer_model_handle, * body)));

		////////////////////////////////////////////////////////////////////////////////
		// ball
		
		std::shared_ptr<Entity> ball_entity;
		gfx::ObjectHandle exception_object;

		if (_player_type == PlayerType::cos_saucer || _player_type == PlayerType::ball_saucer)
		{
			ball_entity = engine.CreateObject<Entity>();

			// physics
			auto ball_body = std::unique_ptr<physics::SphereBody>(
				new physics::SphereBody(transformation, & velocity, physics_engine, saucer_ball_radius));

			// weld UFO's cylinder and ball together to approximate its graphical shape
			ball_body->SetIsCollidable(* body, false);
			body->SetIsCollidable(* ball_body, false);
			physics_engine.Attach(* body, * ball_body);

			if (_player_type == PlayerType::ball_saucer)
			{
				// graphics
				gfx::ObjectHandle ball_model_handle = gfx::BallHandle::Create(transformation, saucer_ball_radius, ufo_color3);
				ball_entity->SetModel(Entity::ModelPtr(new Model(ball_model_handle, * ball_body)));
				
				exception_object = ball_model_handle;
			}
			else
			{
				exception_object = saucer_model_handle;
			}

			ball_entity->SetLocation(std::move(ball_body));
		}

		// assign saucer physics
		ufo_entity.SetLocation(std::move(body));

		////////////////////////////////////////////////////////////////////////////////
		// saucer
		
		// searchlight
		if (saucer_search_light_enable)
		{
			// distance between saucer mesh and ball (in the case of ball saucer);
			// ensures light is clear of enclosing object that would extinguish it
			float clear_distance = (saucer_height * .5f + saucer_ball_radius) * .5f;
			
			gfx::Transformation search_light_transformation(Vector3(0.f, 0.f, - clear_distance));
			gfx::ObjectHandle light_handle = gfx::SearchLightHandle::Create(
				search_light_transformation, 
				gfx::Color4f(.25f, .5f, 1.f) * 20.f, 
				Vector2(std::sin(saucer_search_light_angle), std::cos(saucer_search_light_angle)),
				exception_object);
			gfx::Daemon::Call([light_handle, saucer_model_handle] (gfx::Engine & engine) {
				engine.OnSetParent(light_handle, saucer_model_handle);
			});
		}

		// controller
		std::unique_ptr<Controller> controller;
		switch (ufo_controller_type)
		{
			default:
				DEBUG_BREAK("invalid value; ufo_controller_type:%d; range:[1,2]", ufo_controller_type);
			case 1:
				controller = std::unique_ptr<UfoController1>(
					new UfoController1(ufo_entity, ball_entity, thrust));

				if (SDL_SetRelativeMouseMode(SDL_TRUE))
				{
					// Linux requires libxi-dev to be installed for this to succeed.
					DEBUG_MESSAGE("Failed to set relative mouse mode.");
				}
				break;
			case 2:
				controller = std::unique_ptr<UfoController2>(
					new UfoController2(ufo_entity, ball_entity, thrust));
				break;
		}
		ufo_entity.SetController(std::move(controller));
	}
	
	void AddUfoResources()
	{
		// TODO: When physcis and gfx index are same size, return one resource from the other

		// ship
		crag::GlobalResourceManager::Register<gfx::LitMesh>("ShipLitMesh", [] ()
		{
			return GenerateShipMesh();
		});
		crag::GlobalResourceManager::Register<physics::Mesh>("ShipPhysicsMesh", [] ()
		{
			auto lit_mesh = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("ShipLitMesh");
			return LitToPlainMesh<physics::ElementIndex>(* lit_mesh);
		});
		crag::GlobalResourceManager::Register<gfx::PlainMesh>("ShipShadowMesh", [] ()
		{
			auto lit_mesh = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("ShipLitMesh");
			return LitToPlainMesh<gfx::ElementIndex>(* lit_mesh);
		});
		crag::GlobalResourceManager::Register<gfx::LitMesh>("ShipFlatLitMesh", [] ()
		{
			auto lit_mesh = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("ShipLitMesh");
			return GenerateFlatLitMesh(* lit_mesh);
		});
		
		// cos saucer
		crag::GlobalResourceManager::Register<gfx::LitMesh>("CosSaucerLitMesh", [] ()
		{
			return GenerateCosSaucerMesh(saucer_ball_radius, saucer_radius);
		});
		crag::GlobalResourceManager::Register<physics::Mesh>("CosSaucerPhysicsMesh", [] ()
		{
			auto lit_mesh = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("CosSaucerLitMesh");
			return LitToPlainMesh<physics::ElementIndex>(* lit_mesh);
		});
		crag::GlobalResourceManager::Register<gfx::PlainMesh>("CosSaucerShadowMesh", [] ()
		{
			auto lit_mesh = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("CosSaucerLitMesh");
			return LitToPlainMesh<gfx::ElementIndex>(* lit_mesh);
		});
		crag::GlobalResourceManager::Register<gfx::LitMesh>("CosSaucerFlatLitMesh", [] ()
		{
			auto lit_mesh = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("CosSaucerLitMesh");
			return GenerateFlatLitMesh(* lit_mesh);
		});
		
		// ball saucer
		crag::GlobalResourceManager::Register<gfx::LitMesh>("BallSaucerLitMesh", [] ()
		{
			return GenerateBallSaucerMesh(saucer_height, saucer_radius);
		});
		crag::GlobalResourceManager::Register<gfx::LitMesh>("BallSaucerFlatLitMesh", [] ()
		{
			auto lit_mesh_handle = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("BallSaucerLitMesh");
			return GenerateFlatLitMesh(* lit_mesh_handle);
		});
		crag::GlobalResourceManager::Register<physics::Mesh>("BallSaucerPhysicsMesh", [] ()
		{
			auto lit_mesh = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("BallSaucerLitMesh");
			return LitToPlainMesh<physics::ElementIndex>(* lit_mesh);
		});
		crag::GlobalResourceManager::Register<gfx::PlainMesh>("BallSaucerShadowMesh", [] ()
		{
			auto lit_mesh = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("BallSaucerLitMesh");
			return LitToPlainMesh<gfx::ElementIndex>(* lit_mesh);
		});
		
		// thargoid
		crag::GlobalResourceManager::Register<gfx::LitMesh>("ThargoidLitMesh", [] ()
		{
			return GenerateThargoidMesh(thargoid_height, thargoid_radius);
		});
		crag::GlobalResourceManager::Register<gfx::LitMesh>("ThargoidFlatLitMesh", [] ()
		{
			auto lit_mesh_handle = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("ThargoidLitMesh");
			return GenerateFlatLitMesh(* lit_mesh_handle);
		});
		crag::GlobalResourceManager::Register<physics::Mesh>("ThargoidPhysicsMesh", [] ()
		{
			auto lit_mesh = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("ThargoidLitMesh");
			return LitToPlainMesh<physics::ElementIndex>(* lit_mesh);
		});
		crag::GlobalResourceManager::Register<gfx::PlainMesh>("ThargoidShadowMesh", [] ()
		{
			auto lit_mesh = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("ThargoidLitMesh");
			return LitToPlainMesh<gfx::ElementIndex>(* lit_mesh);
		});
		
		gfx::Daemon::Call([] (gfx::Engine & engine)
		{
			auto & resource_manager = engine.GetResourceManager();
			
			resource_manager.Register<gfx::LitVboResource>("ShipVbo", [] ()
			{
				auto lit_mesh_handle = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("ShipFlatLitMesh");
				return gfx::LitVboResource(* lit_mesh_handle);
			});

			resource_manager.Register<gfx::LitVboResource>("CosSaucerVbo", [] ()
			{
				auto lit_mesh_handle = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("CosSaucerLitMesh");
				return gfx::LitVboResource(* lit_mesh_handle);
			});

			resource_manager.Register<gfx::LitVboResource>("CosSaucerFlatLitVbo", [] ()
			{
				auto lit_mesh_handle = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("CosSaucerFlatLitMesh");
				return gfx::LitVboResource(* lit_mesh_handle);
			});

			resource_manager.Register<gfx::LitVboResource>("BallSaucerVbo", [] ()
			{
				auto lit_mesh_handle = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("BallSaucerLitMesh");
				return gfx::LitVboResource(* lit_mesh_handle);
			});

			resource_manager.Register<gfx::LitVboResource>("BallSaucerFlatLitVbo", [] ()
			{
				auto lit_mesh_handle = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("BallSaucerFlatLitMesh");
				return gfx::LitVboResource(* lit_mesh_handle);
			});

			resource_manager.Register<gfx::LitVboResource>("ThargoidVbo", [] ()
			{
				auto lit_mesh_handle = crag::GlobalResourceManager::GetHandle<gfx::LitMesh>("ThargoidFlatLitMesh");
				return gfx::LitVboResource(* lit_mesh_handle);
			});
		
		});
	}
}

EntityHandle SpawnRover(Vector3 const & position, Scalar thrust)
{
	auto vehicle = EntityHandle::Create();

	Sphere3 sphere;
	sphere.center = static_cast<Vector3>(position);
	sphere.radius = 1.;

	vehicle.Call([sphere, thrust] (Entity & entity) {
		ConstructRover(entity, sphere, thrust);
	});

	return vehicle;
}

std::array<sim::EntityHandle, 2> SpawnPlayer(sim::Vector3 const & position, sim::Vector3 const & forward, geom::Space const & space)
{
	AddUfoResources();
	
	auto player = EntityHandle::Create();
	auto up = geom::Normalized(position - space.AbsToRel(geom::uni::Vector3::Zero()));

	auto _player_type = PlayerType(player_type);
	player.Call([=] (Entity & entity) {
		switch (_player_type)
		{
			case PlayerType::observer:
			{
				sim::Transformation transformation(
					position,
					geom::Rotation(forward, up, geom::Direction::forward, geom::Direction::up));
				ConstructObserver(entity, transformation);
				break;
			}

			case PlayerType::arrow:
			{
				ConstructShip(entity, position);
				break;
			}

			case PlayerType::thargoid:
			{
				sim::Transformation transformation(position, geom::Rotation(up, geom::Direction::forward));
				ConstructUfo(
					entity, transformation,
					"ThargoidVbo", "ThargoidShadowMesh",
					_player_type,
					thargoid_thrust, thargoid_radius);
				break;
			}

			case PlayerType::cos_saucer:
			{
				sim::Transformation transformation(position, geom::Rotation(up, geom::Direction::forward));
				ConstructUfo(
					entity, transformation,
					saucer_flat_shade_cos ? "CosSaucerFlatLitVbo" : "CosSaucerVbo", "CosSaucerShadowMesh",
					_player_type,
					saucer_thrust, saucer_radius);
				break;
			}

			case PlayerType::ball_saucer:
			{
				sim::Transformation transformation(position, geom::Rotation(up, geom::Direction::forward));
				ConstructUfo(
					entity, transformation,
					saucer_flat_shade_ball ? "BallSaucerFlatLitVbo" : "BallSaucerVbo", "BallSaucerShadowMesh",
					_player_type,
					saucer_thrust, saucer_radius);
				break;
			}
		}
	});

	// Create camera.
	sim::EntityHandle camera;
	if (_player_type != PlayerType::observer)
	{
		camera = SpawnCamera(position + camera_start_offset, player);
	}

	return {{ player, camera }};
}
