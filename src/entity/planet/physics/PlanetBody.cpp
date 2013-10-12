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

#include "physics/Engine.h"
#include "physics/MeshSurround.h"
#include "physics/RayCast.h"

#include "form/CastRay.h"
#include "form/ForEachFaceInSphere.h"

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// PlanetBody members

DEFINE_POOL_ALLOCATOR(PlanetBody, 3);

PlanetBody::PlanetBody(Transformation const & transformation, Engine & engine, form::Polyhedron const & polyhedron, Scalar radius)
: SphericalBody(transformation, nullptr, engine, radius)
, _polyhedron(polyhedron)
, _mean_radius(radius)
{
}

void PlanetBody::GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const
{
	Vector3 const & center = GetTranslation();
	Vector3 to_center = center - pos;
	Scalar distance = Length(to_center);
	
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

	Vector3 contribution = direction * force;
	gravity += contribution;
}

bool PlanetBody::OnCollision(Body & body, ContactInterface & contact_interface)
{
	return body.OnCollision(* this, contact_interface);
}

bool PlanetBody::OnCollisionWithSolid(Body & body, Sphere3 const & bounding_sphere, ContactInterface & contact_interface)
{
	////////////////////////////////////////////////////////////////////////////////
	// gather the two collision handles together

	// the incoming body - could be any class of shape
	auto body_collision_handle = body.GetCollisionHandle();
	auto planet_collision_handle = GetCollisionHandle();
	
	auto & mesh_surround = _engine.GetMeshSurround(planet_collision_handle, body_collision_handle);
	auto mesh_collision_handle = mesh_surround.GetCollisionHandle();
	
	////////////////////////////////////////////////////////////////////////////////
	// generate mesh representing the planet surface in the vacinity of the body

	mesh_surround.ClearData();

	// only applied if the body is embedded and won't register with ODE collision
	Scalar max_depth = std::numeric_limits<Scalar>::lowest();
	Vector3 max_depth_normal;
	auto face_functor = [&] (form::Triangle3 const & face, form::Vector3 const & normal)
	{
		Vector3 center = geom::Center(face);
		form::Plane3 plane(center, normal);
		
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
		
		mesh_surround.AddTriangle(face, normal);
	};
	
	form::ForEachFaceInSphere(_polyhedron, bounding_sphere, face_functor);
	
    ////////////////////////////////////////////////////////////////////////////////
	// collide and generate contacts

	constexpr auto max_num_contacts = 10240;
	typedef std::array<ContactGeom, max_num_contacts> ContactVector;
	ContactVector contacts;
	std::size_t num_contacts;
	
	if (! mesh_surround.IsEmpty())
	{
		mesh_surround.RefreshData();
		mesh_surround.Enable();
	
		int flags = contacts.size() - 1;
		ASSERT((flags >> 16) == 0);
	
		num_contacts = dCollide(body_collision_handle, mesh_collision_handle, flags, contacts.data(), sizeof(ContactVector::value_type));

		ASSERT(num_contacts <= contacts.size());
	}
	else
	{
		num_contacts = 0;
	}
	
	// If there's a good chance the body is contained by the polyhedron,
	if (max_depth > 0)
	{
		// add a provisional contact.
		auto & containment_geom = contacts[num_contacts ++];
		
		Convert(containment_geom.pos, bounding_sphere.center);
		containment_geom.normal[0] = max_depth_normal.x;
		containment_geom.normal[1] = max_depth_normal.y;
		containment_geom.normal[2] = max_depth_normal.z;
		containment_geom.depth = max_depth;
		containment_geom.g1 = body_collision_handle;
		containment_geom.g2 = mesh_collision_handle;
	}

	// If contact was detected,
	if (num_contacts != 0)
	{
		// add it to the list to be resolved.
		auto begin = contacts.data();
		contact_interface(begin, begin + num_contacts);
	}

	////////////////////////////////////////////////////////////////////////////////
	// reset
	
	mesh_surround.Disable();
	
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
			VerifyOp(length_to_cast, <=, ray_cast.GetLength());

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
