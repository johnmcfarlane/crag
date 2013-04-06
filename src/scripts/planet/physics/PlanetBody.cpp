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
#include "physics/IntersectionFunctor.h"

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
	
	CONFIG_DEFINE (formation_sphere_collision_detail_factor, float, 1.f);
	CONFIG_DEFINE (formation_box_collision_detail_factor, float, 1.5f);
	
	
	////////////////////////////////////////////////////////////////////////////////
	// DeferredIntersectionFunctor - wrapper for TreeQueryFunctor
	
	class DeferredIntersectionFunctor : public smp::scheduler::Job
	{
	public:	
		DeferredIntersectionFunctor(Body const & body, PlanetBody const & planet_body, IntersectionFunctor const & intersection_functor)
		: _body(body)
		, _planet_body(planet_body)
		, _intersection_functor(intersection_functor)
		{
		}
		
	private:
		virtual void operator () (size_type unit_index)
		{
			ASSERT(unit_index == 0);
			
			_body.OnDeferredCollisionWithPlanet(_planet_body, _intersection_functor);
		}
		
		Body const & _body;
		PlanetBody const & _planet_body;
		IntersectionFunctor _intersection_functor;
	};
}


////////////////////////////////////////////////////////////////////////////////
// PlanetBody members

DEFINE_POOL_ALLOCATOR(PlanetBody, 3);

PlanetBody::PlanetBody(Engine & physics_engine, form::Formation const & formation, Scalar radius)
: SphericalBody(physics_engine, false, radius)
, _formation(formation)
, _mean_radius(radius)
{
}

void PlanetBody::GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const
{
	Vector3 const & center = GetPosition();
	Vector3 to_center = center - pos;
	Scalar distance = Length(to_center);
	
	// Calculate the direction of the pull.
	Vector3 direction = to_center / distance;

	// Calculate the mass.
	Scalar density = 1;
	Scalar volume = Cube(_mean_radius);
	Scalar mass = volume * density;

	// Calculate the force. Actually, this isn't really the force;
	// It's the potential. Until we know what we're pulling we can't know the force.
	Scalar force;
	if (distance < _mean_radius)
	{
		force = mass * distance / Cube(_mean_radius);
	}
	else
	{
		force = mass / Square(distance);
	}

	Vector3 contribution = direction * force;
	gravity += contribution;
}

bool PlanetBody::OnCollision(Engine & engine, Body const & that_body) const
{
	CollisionHandle that_collision_handle = that_body.GetCollisionHandle();
	CollisionHandle this_collision_handle = GetCollisionHandle();
	IntersectionFunctor intersection_functor(engine, that_collision_handle, this_collision_handle);
	
	that_body.OnDeferredCollisionWithPlanet(* this, intersection_functor);
	
	return true;
}

void PlanetBody::OnDeferredCollisionWithBox(Body const & body, IntersectionFunctor & functor) const
{
	using namespace form::collision;

	BoxBody const & box = static_cast<BoxBody const &>(body);
	Engine const & physics_engine = functor.GetEngine();
	form::Scene const & scene = physics_engine.GetScene();
	
	// Get vital geometric information about the cuboid.
	Vector3 position = box.GetPosition();
	Vector3 dimensions = box.GetScale();
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
	form::Polyhedron const * polyhedron = scene.GetPolyhedron(_formation);
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

void PlanetBody::OnDeferredCollisionWithSphere(Body const & body, IntersectionFunctor & functor) const
{
	using namespace form::collision;

	SphericalBody const & sphere = static_cast<SphericalBody const &>(body);
	Engine const & physics_engine = functor.GetEngine();
	form::Scene const & scene = physics_engine.GetScene();

	form::Polyhedron const * polyhedron = scene.GetPolyhedron(_formation);
	if (polyhedron == nullptr)
	{
		// This can happen if the PlanetBody has just been created 
		// and the corresponding OnAddFormation message hasn't been read yet.
		return;
	}
	
	typedef Object<form::Sphere3> Object;
	Object collision_object;
	collision_object.bounding_sphere.center = sphere.GetPosition();
	collision_object.bounding_sphere.radius = sphere.GetRadius();
	
	collision_object.shape = collision_object.bounding_sphere;
	
	form::Vector3 const & relative_formation_position = geom::Cast<form::Scalar>(polyhedron->GetShape().center);
	float sphere_area(Area(collision_object.shape));
	float min_parent_area = sphere_area * formation_sphere_collision_detail_factor;

	ForEachCollision(* polyhedron, relative_formation_position, collision_object, functor, min_parent_area);
}
