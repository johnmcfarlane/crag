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

#include "form/scene/ForEachFaceInSphere.h"
#include "form/scene/Scene.h"

#include "core/ConfigEntry.h"

using namespace physics;

namespace
{
	
	////////////////////////////////////////////////////////////////////////////////
	// config constants
	
	CONFIG_DEFINE(planet_collision_friction, physics::Scalar, .1f);	// coulomb friction coefficient
	CONFIG_DEFINE(planet_collision_bounce, physics::Scalar, .50);
}

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

bool PlanetBody::OnCollision(Engine & engine, Body const & that_body) const
{
	CollisionHandle that_collision_handle = that_body.GetCollisionHandle();
	CollisionHandle this_collision_handle = GetCollisionHandle();

	dContact contact;
	ZeroObject(contact);
	contact.surface.mode = dContactBounce | dContactSlip1 | dContactSlip2;
	contact.surface.mu = planet_collision_friction;
	contact.surface.bounce = planet_collision_bounce;
	contact.surface.bounce_vel = .1f;
	contact.geom.g1 = that_collision_handle;
	contact.geom.g2 = this_collision_handle;
	
	auto f = [&engine, &contact] (Vector3 const & pos, Vector3 const & normal, Scalar depth) {
		contact.geom.pos[0] = pos.x;
		contact.geom.pos[1] = pos.y;
		contact.geom.pos[2] = pos.z;
		contact.geom.normal[0] = normal.x;
		contact.geom.normal[1] = normal.y;
		contact.geom.normal[2] = normal.z;
		contact.geom.depth = depth;
	
		engine.OnContact(contact);
	};

	that_body.OnDeferredCollisionWithPlanet(* this, physics::IntersectionFunctorRef(f));
	
	return true;
}

void PlanetBody::OnDeferredCollisionWithBox(Body const & body, IntersectionFunctorRef const & functor) const
{
	OnDeferredCollisionWithSimpleBody(body, functor);
}

void PlanetBody::OnDeferredCollisionWithRay(Body const & body, IntersectionFunctorRef const & functor) const
{
	OnDeferredCollisionWithSimpleBody(body, functor);
}

void PlanetBody::OnDeferredCollisionWithSphere(Body const & body, IntersectionFunctorRef const & functor) const
{
	OnDeferredCollisionWithSimpleBody(body, functor);
}

void PlanetBody::OnDeferredCollisionWithSimpleBody(Body const & body, IntersectionFunctorRef const & functor) const
{
	////////////////////////////////////////////////////////////////////////////////
	// get necessary data for scanning planet surface

	auto & scene = _engine.GetScene();
	auto polyhedron = scene.GetPolyhedron(_formation);
	if (polyhedron == nullptr)
	{
		// This can happen if the PlanetBody has just been created 
		// and the corresponding OnAddFormation message hasn't been read yet.
		return;
	}
	
	////////////////////////////////////////////////////////////////////////////////
	// gather the two collision handles together

	// the incoming body - could be any class of shape
	auto body_collision_handle = body.GetCollisionHandle();
	auto planet_collision_handle = GetCollisionHandle();
	
	auto & mesh_surround = _engine.GetMeshSurround(planet_collision_handle, body_collision_handle);
	auto mesh_collision_handle = mesh_surround.GetCollisionHandle();
	mesh_surround.Enable();
	
	////////////////////////////////////////////////////////////////////////////////
	// generate mesh representing the planet surface in the vacinity of the body

	form::Sphere3 collision_sphere = body.GetBoundingSphere();
	mesh_surround.ClearData();

	auto face_functor = [& mesh_surround] (form::Point & a, form::Point & b, form::Point & c, geom::Vector3f const & normal, float /*score*/)
	{
		mesh_surround.AddTriangle(a.pos, b.pos, c.pos, normal);
	};
	
	form::ForEachFaceInSphere(* polyhedron, collision_sphere, face_functor);
	
	mesh_surround.RefreshData();
	
    ////////////////////////////////////////////////////////////////////////////////
	// collide and generate contacts

	constexpr auto max_num_contacts = 10240;
	typedef std::array<dContactGeom, max_num_contacts> ContactVector;
	ContactVector contacts;
	
	int flags = contacts.size();
	ASSERT((flags >> 16) == 0);
	ASSERT(flags >= max_num_contacts);
	
	std::size_t num_contacts = dCollide(body_collision_handle, mesh_collision_handle, flags, contacts.data(), sizeof(ContactVector::value_type));
	ASSERT(num_contacts <= contacts.size());
	
	////////////////////////////////////////////////////////////////////////////////
	// execute

	for (auto index = 0u; index < num_contacts; ++ index)
	{
		dContactGeom const & contact_geom = contacts[index];
		
		Vector3 position(contact_geom.pos[0], contact_geom.pos[1], contact_geom.pos[2]);	// TODO: Nifty conversion hacks between dVector and Vector3f
		Vector3 normal(contact_geom.normal[0], contact_geom.normal[1], contact_geom.normal[2]);
		Scalar depth = contact_geom.depth;
		ASSERT(contact_geom.g1 == body_collision_handle);
		ASSERT(contact_geom.g2 == mesh_collision_handle);

		functor(position, normal, depth);
	}
	
	////////////////////////////////////////////////////////////////////////////////
	// reset
	
	mesh_surround.Disable();
}
