//
//  PlanetBody.cpp
//  crag
//
//  Created by John on 6/20/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "PlanetBody.h"

#include "physics/RayCast.h"

#include "form/CastRay.h"
#include "form/ForEachFaceInSphere.h"

#include "gfx/Debug.h"
#include "gfx/Mesh.h"
#include "gfx/PlainVertex.h"

#include "core/Random.h"

#include "geom/Vector.h"

using namespace physics;

// std::hash<form::Triangle3 const &
namespace std
{
	// for file-local use only; not a general solution:
	// yields the same value for same combination of components in any order
	template<>
	struct hash<form::Triangle3>
	{
		typedef form::Triangle3 argument_type;
		typedef std::size_t result_type;

		result_type operator()(argument_type const & triangle) const
		{
			auto seed = size_t(0);

			for (auto & point : triangle.points)
			{
				for (auto component = ::begin(point); component != ::end(point); ++ component)
				{
					auto hash = std::hash<Scalar>()(* component);
					seed = crag::core::hash_combine(seed, hash);
				}
			}

			return seed;
		}
	};
}

////////////////////////////////////////////////////////////////////////////////
// PlanetBody members

PlanetBody::PlanetBody(Transformation const & transformation, Engine & engine, form::Polyhedron const & polyhedron, Scalar radius)
: SphereBody(transformation, nullptr, engine, radius)
, _polyhedron(polyhedron)
, _mean_radius(radius)
{
}

Vector3 PlanetBody::GetGravitationalAttraction(Vector3 const & pos) const
{
	Vector3 const & center = GetTranslation();
	Vector3 to_center = center - pos;
	Scalar distance = Magnitude(to_center);
	
	// Calculate the direction of the pull.
	Vector3 direction = to_center / distance;

	// Calculate the mass.
	Scalar density = 1;
	Scalar volume = geom::Sphere<Scalar, 3>::Properties::Volume(_mean_radius);
	Scalar mass = volume * density;

	// Calculate the force. Actually, this isn't really the force;
	// It's the potential. Until we know what we're pulling we can't know the force.
	Scalar force;
	if (distance < _mean_radius)
	{
		force = mass * distance / Cubed(_mean_radius);
	}
	else
	{
		force = mass / Squared(distance);
	}

	return direction * force;
}

bool PlanetBody::OnCollision(Body & body, ContactFunction & contact_function)
{
	// TODO: planet-to-planet collision causes stack overflow
	return body.OnCollision(* this, contact_function);
}

bool PlanetBody::OnCollisionWithSolid(Body & body, Sphere3 const & bounding_sphere, ContactFunction & contact_function)
{
	////////////////////////////////////////////////////////////////////////////////
	// gather the two collision handles together

	// the incoming body - could be any class of shape
	auto body_collision_handle = body.GetCollisionHandle();
	auto planet_collision_handle = GetCollisionHandle();
	
	////////////////////////////////////////////////////////////////////////////////
	// generate mesh representing the planet surface in the vacinity of the body

	// TODO: make thread-safe; this is low-priority because:
	// a) using single-threaded collision;
	// b) thread-safety strategy is likely to depend on specifics of parallelization
	static Mesh mesh;
	static std::vector<Vector3> normals;

	auto & vertices = mesh.GetVertices();
	auto & indices = mesh.GetIndices();
	
	ASSERT(vertices.empty());
	ASSERT(indices.empty());
	ASSERT(normals.empty());

	// only applied if the body is embedded and won't register with ODE collision
	Scalar max_depth = std::numeric_limits<Scalar>::lowest();
	Vector3 max_depth_normal;
	auto face_functor = [&] (form::Triangle3 const & face, form::Vector3 const & normal)
	{
		Vector3 centroid = geom::Centroid(face);
		form::Plane3 plane(centroid, normal);
		
		auto distance = Distance(plane, bounding_sphere.center);
		if (distance > bounding_sphere.radius)
		{
			// body is clear of this poly - even if it was an infinite plane
			return;
		}
		
		auto depth = bounding_sphere.radius - distance;
		if (depth > max_depth)
		{
			max_depth = depth;
			max_depth_normal = normal;
		}
		
		auto index = vertices.size() + 2;
		for (auto & point : face.points)
		{
			indices.push_back(index --);
			vertices.push_back(gfx::PlainVertex{ point });
			normals.push_back(normal);
		}
	};
	
	form::ForEachFaceInSphere(_polyhedron, bounding_sphere, face_functor);
	
	// early-out
	if (indices.empty())
	{
		return true;
	}
	
    ////////////////////////////////////////////////////////////////////////////////
	// create physics geometry based on mesh

	MeshData mesh_data = dGeomTriMeshDataCreate();
	dGeomTriMeshDataBuildSingle1(mesh_data,
		vertices.front().pos.GetAxes(), sizeof(Mesh::value_type), vertices.size(),
		indices.data(), indices.size(), sizeof(Mesh::index_type),
		reinterpret_cast<int *>(normals.data()));

	CollisionHandle mesh_collision_handle = dCreateTriMesh(nullptr, mesh_data, nullptr, nullptr, nullptr);
	
    ////////////////////////////////////////////////////////////////////////////////
	// collide and generate contacts

	constexpr auto max_num_contacts = 10240;
	constexpr auto spare_contact = 1;
	typedef std::array<ContactGeom, max_num_contacts> ContactVector;
	ContactVector contacts;
	std::size_t num_contacts = 0;
	
	int flags = contacts.size() - spare_contact;
	ASSERT((flags >> 16) == 0);

	num_contacts = dCollide(body_collision_handle, mesh_collision_handle, flags, contacts.data(), sizeof(ContactVector::value_type));

	ASSERT(num_contacts <= contacts.size());
	
	// If there's a good chance the body is contained by the polyhedron,
	if (num_contacts == 0 && max_depth > bounding_sphere.radius * 2.f)
	{
		// add a provisional contact.
		auto & containment_geom = contacts[num_contacts];
		num_contacts += spare_contact;
		
		Convert(containment_geom.pos, bounding_sphere.center);
		containment_geom.normal[0] = max_depth_normal.x;
		containment_geom.normal[1] = max_depth_normal.y;
		containment_geom.normal[2] = max_depth_normal.z;
		containment_geom.depth = max_depth;
		containment_geom.g1 = body_collision_handle;
		containment_geom.g2 = planet_collision_handle;
		
		CRAG_VERIFY_OP(max_depth, >=, 0);
		CRAG_VERIFY_OP(containment_geom.depth, >=, 0);
	}
	else
	{
		// switcheroo - replace temporary mesh id with permanent planet id
		std::for_each(std::begin(contacts), std::begin(contacts) + num_contacts, [&] (ContactGeom & contact)
		{
			ASSERT(contact.g1 = body_collision_handle);
			ASSERT(contact.g2 = mesh_collision_handle);
			contact.g2 = planet_collision_handle;
		});
	}

	// If contact was detected,
	if (num_contacts != 0)
	{
		// add it to the list to be resolved.
		auto begin = contacts.data();
		contact_function(begin, begin + num_contacts);
	}

	////////////////////////////////////////////////////////////////////////////////
	// clean up physics objects
	
	dGeomDestroy(mesh_collision_handle);
	dGeomTriMeshDataDestroy(mesh_data);

	////////////////////////////////////////////////////////////////////////////////
	// clean up mesh
	
	vertices.clear();
	indices.clear();
	normals.clear();

	return true;
}

bool PlanetBody::OnCollisionWithRay(Body & body)
{
	auto & ray_cast = static_cast<RayCast &>(body);

	auto calculate_length_to_cast = [& ray_cast] ()
	{
		const auto & previous_ray_casy_result = ray_cast.GetResult();
		if (previous_ray_casy_result)
		{
			// if contact has previously been detected,
			// testing beyond the distance of that contact is of no use
			auto length_to_cast = previous_ray_casy_result.GetDistance();

			// check that this is indeed a shortening of the ray to cast
			CRAG_VERIFY_OP(length_to_cast, <=, ray_cast.GetLength());

			return length_to_cast;
		}
		else
		{
			return ray_cast.GetLength();
		}
	};

	const auto length = calculate_length_to_cast();
	const auto ray = ray_cast.GetRay();

	////////////////////////////////////////////////////////////////////////////////
	// test ray against polyhedron and register result with ray_cast

	const auto ray_cast_result = form::CastRay(_polyhedron, ray, length);

	ray_cast.SampleResult(ray_cast_result);
	return true;
}

void PlanetBody::DebugDraw() const
{
	using namespace gfx::Debug;

	auto bounding_sphere = Sphere3(GetTranslation(), GetRadius() * .001f);
	bounding_sphere.center = Vector3(0.f, 0.f, 0.f);


	auto face_functor = [] (form::Triangle3 const & face, form::Vector3 const &)
	{
		Random s(std::hash<form::Triangle3>()(face));

		auto r = [& s] () { return s.GetUnit<float>(); };
		auto color = Color(r(), r(), r());

		AddTriangle(face, ColorPair(color, color * .25f));
	};

	form::ForEachFaceInSphere(_polyhedron, bounding_sphere, face_functor);
}
