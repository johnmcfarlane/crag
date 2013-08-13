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

#include "sim/defs.h"
#include "sim/Engine.h"

#include "physics/BoxBody.h"
#include "physics/Engine.h"

#include "form/Formation.h"
#include "form/Engine.h"
#include "form/node/NodeBuffer.h"
#include "form/scene/collision.h"

#include "geom/MatrixOps.h"

#include "core/ConfigEntry.h"

using namespace physics;

namespace
{
	
	////////////////////////////////////////////////////////////////////////////////
	// config constants
	
	CONFIG_DEFINE(formation_sphere_collision_detail_factor, float, 1.f);
	CONFIG_DEFINE(formation_box_collision_detail_factor, float, 1.5f);
	CONFIG_DEFINE(planet_collision_friction, physics::Scalar, .1f);	// coulomb friction coefficient
	CONFIG_DEFINE(planet_collision_bounce, physics::Scalar, .50);
}


////////////////////////////////////////////////////////////////////////////////
// PlanetBody members

DEFINE_POOL_ALLOCATOR(PlanetBody, 3);

PlanetBody::PlanetBody(Transformation const & transformation, Engine & physics_engine, form::Formation const & formation, Scalar radius)
: SphericalBody(transformation, nullptr, physics_engine, radius)
, _formation(formation)
, _scene(physics_engine.GetScene())
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
	dGeomID object_geom = that_body.GetGeomId();
	dGeomID planet_geom = GetGeomId();

	dContact contact;
	ZeroObject(contact);
	contact.surface.mode = dContactBounce | dContactSlip1 | dContactSlip2;
	contact.surface.mu = planet_collision_friction;
	contact.surface.bounce = planet_collision_bounce;
	contact.surface.bounce_vel = .1f;
	contact.geom.g1 = object_geom;
	contact.geom.g2 = planet_geom;
	
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
	using namespace form::collision;

	BoxBody const & box = static_cast<BoxBody const &>(body);
	
	// Get vital geometric information about the cuboid.
	Vector3 position = box.GetTranslation();
	Vector3 dimensions = box.GetDimensions();
	Vector3 extents = dimensions * Scalar(.5);
	Matrix33 rotation = box.GetRotation();
	
	// Initialise the PointCloud.
	typedef Object<ConvexHull> Object;
	Object collision_object;
	
	// bounding sphere
	collision_object.bounding_sphere.center = position;
	collision_object.bounding_sphere.radius = Length(extents);
	
	// points
	{
		ConvexHull::Vector::iterator i = collision_object.shape.faces.begin();
		for (int axis = 0; axis < 3; ++ axis)
		{
			for (int pole = 0; pole < 2; ++ pole)
			{
				float pole_sign = pole ? 1.f : -1.f;
				sim::Ray3 plane;
				
				plane.position = Vector3::Zero();
				plane.position[axis] = pole_sign * extents[axis];
				plane.position = rotation * plane.position;
				i->position = plane.position;
				i->position += collision_object.bounding_sphere.center;
				
				plane.direction = Vector3::Zero();
				plane.direction[axis] = pole_sign;
				plane.direction = rotation * plane.direction;
				i->direction = plane.direction;
				
				//gfx::Debug::AddLine(form::SceneToSim(sim::Vector3(i->position), origin), 
				//	form::SceneToSim(sim::Vector3(i->position + i->direction), origin),
				//	gfx::Debug::ColorPair(gfx::Color4f::White(), 0.1f));
				
				++ i;
			}
		}
		ASSERT(i == collision_object.shape.faces.end());
	}
 	
	// TODO: Try and move as much of this as possible into the ForEachIntersection fn.
	form::Polyhedron const * polyhedron = _scene.GetPolyhedron(_formation);
	if (polyhedron == nullptr)
	{
		ASSERT(false);
		return;
	}
	
	form::Vector3 const & relative_formation_position = geom::Cast<form::Scalar>(polyhedron->GetShape().center);
	float min_box_edge = std::min(float(dimensions.x), std::min(float(dimensions.y), float(dimensions.z)));
	float min_parent_area = min_box_edge * formation_box_collision_detail_factor;
	
	ForEachCollision(* polyhedron, relative_formation_position, collision_object, functor, min_parent_area);
}

void PlanetBody::OnDeferredCollisionWithSphere(Body const & body, IntersectionFunctorRef const & functor) const
{
	using namespace form::collision;

	SphericalBody const & sphere = static_cast<SphericalBody const &>(body);

	form::Polyhedron const * polyhedron = _scene.GetPolyhedron(_formation);
	if (polyhedron == nullptr)
	{
		// This can happen if the PlanetBody has just been created 
		// and the corresponding OnAddFormation message hasn't been read yet.
		return;
	}
	
	typedef Object<form::Sphere3> Object;
	Object collision_object;
	collision_object.bounding_sphere.center = sphere.GetTranslation();
	collision_object.bounding_sphere.radius = sphere.GetRadius();
	
	collision_object.shape = collision_object.bounding_sphere;
	
	form::Vector3 const & relative_formation_position = geom::Cast<form::Scalar>(polyhedron->GetShape().center);
	float sphere_area(Area(collision_object.shape));
	float min_parent_area = sphere_area * formation_sphere_collision_detail_factor;

	ForEachCollision(* polyhedron, relative_formation_position, collision_object, functor, min_parent_area);
}
