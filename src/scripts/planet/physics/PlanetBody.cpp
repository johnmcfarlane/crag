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
#include "form/Scene.h"

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// PlanetBody members

DEFINE_POOL_ALLOCATOR(PlanetBody, 3);

PlanetBody::PlanetBody(Transformation const & transformation, Engine & engine, form::Formation const & formation, Scalar radius)
: SphericalBody(transformation, nullptr, engine, radius)
, _formation(formation)
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

bool PlanetBody::OnCollision(Body & body)
{
	return body.OnCollision(* this);
}

bool PlanetBody::OnCollisionWithSolid(Body & body, Sphere3 const & bounding_sphere)
{
	////////////////////////////////////////////////////////////////////////////////
	// get necessary data for scanning planet surface

	auto & scene = _engine.GetScene();
	auto polyhedron = scene.GetPolyhedron(_formation);
	if (polyhedron == nullptr)
	{
		// This can happen if the PlanetBody has just been created 
		// and the corresponding OnAddFormation message hasn't been read yet.
		return true;
	}
	
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

	bool contained = false;
	auto face_functor = [& mesh_surround, & contained, & bounding_sphere] (form::Triangle3 const & face, form::Vector3 const & normal)
	{
		Vector3 center = geom::Center(face);
		form::Plane3 plane(center, normal);
		
		auto distance = Distance(plane, bounding_sphere.center);
		if (distance > bounding_sphere.radius)
		{
			return;
		}
		
		// lets be kind and call this a heuristic
		if (distance < 0.f)
		{
			contained = true;
		}
		
		mesh_surround.AddTriangle(face, normal);
	};
	
	form::ForEachFaceInSphere(* polyhedron, bounding_sphere, face_functor);
	
	if (mesh_surround.IsEmpty())
	{
		return true;
	}
	
	mesh_surround.RefreshData();
	mesh_surround.Enable();
	
    ////////////////////////////////////////////////////////////////////////////////
	// collide and generate contacts

	constexpr auto max_num_contacts = 10240;
	typedef std::array<ContactGeom, max_num_contacts> ContactVector;
	ContactVector contacts;
	
	int flags = contacts.size();
	ASSERT((flags >> 16) == 0);
	ASSERT(flags >= max_num_contacts);
	
	std::size_t num_contacts = dCollide(body_collision_handle, mesh_collision_handle, flags, contacts.data(), sizeof(ContactVector::value_type));
	ASSERT(num_contacts <= contacts.size());
	
	if (num_contacts != 0)
	{
		auto begin = std::begin(contacts);
		_engine.AddContacts(begin, begin + num_contacts);
	}
	else
	{
		if (contained)
		{
			ContactGeom contact_geom;
			contact_geom.pos[0] = bounding_sphere.center.x;
			contact_geom.pos[1] = bounding_sphere.center.y;
			contact_geom.pos[2] = bounding_sphere.center.z;
			auto normal = geom::Normalized(bounding_sphere.center - geom::Cast<Scalar>(polyhedron->GetShape().center));
			contact_geom.normal[0] = normal.x;
			contact_geom.normal[1] = normal.y;
			contact_geom.normal[2] = normal.z;
			contact_geom.depth = geom::Diameter(bounding_sphere);
			contact_geom.g1 = body_collision_handle;
			contact_geom.g2 = mesh_collision_handle;
			_engine.AddContact(contact_geom);
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////
	// reset
	
	mesh_surround.Disable();
	
	return true;
}

bool PlanetBody::OnCollisionWithRay(Body & body)
{
	auto & ray_cast = static_cast<RayCast &>(body);
	auto ray = ray_cast.GetRay();
	auto length = ray_cast.GetLength();

	////////////////////////////////////////////////////////////////////////////////
	// get necessary data for scanning planet surface

	auto & scene = _engine.GetScene();
	auto polyhedron = scene.GetPolyhedron(_formation);
	if (polyhedron == nullptr)
	{
		// This can happen if the PlanetBody has just been created 
		// and the corresponding OnAddFormation message hasn't been read yet.
		return true;
	}
	
	////////////////////////////////////////////////////////////////////////////////
	// test ray against polyhedron and register result with ray_cast

	auto ray_cast_result = form::CastRay(* polyhedron, ray, length);
	if (ray_cast_result.projection >= 0)
	{
		ray_cast.SampleContact(ray_cast_result.projection);
	}
	
	return true;
}
