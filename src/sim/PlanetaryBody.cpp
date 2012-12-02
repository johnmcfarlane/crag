//
//  PlanetaryBody.cpp
//  crag
//
//  Created by John on 6/20/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "PlanetaryBody.h"

#include "defs.h"
#include "Engine.h"

#include "physics/BoxBody.h"
#include "physics/Engine.h"
#include "physics/IntersectionFunctor.h"

#include "form/Formation.h"
#include "form/Engine.h"
#include "form/node/NodeBuffer.h"
#include "form/scene/collision.h"

#include "geom/MatrixOps.h"

#include "core/ConfigEntry.h"

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
		DeferredIntersectionFunctor(physics::Body const & body, sim::PlanetaryBody const & planetary_body, physics::IntersectionFunctor const & intersection_functor)
		: _body(body)
		, _planetary_body(planetary_body)
		, _intersection_functor(intersection_functor)
		{
		}
		
	private:
		virtual void operator () (size_type unit_index)
		{
			ASSERT(unit_index == 0);
			
			_body.OnDeferredCollisionWithPlanet(_planetary_body, _intersection_functor);
		}
		
		physics::Body const & _body;
		sim::PlanetaryBody const & _planetary_body;
		physics::IntersectionFunctor _intersection_functor;
	};
}


////////////////////////////////////////////////////////////////////////////////
// PlanetaryBody members

sim::PlanetaryBody::PlanetaryBody(physics::Engine & physics_engine, form::Formation const & formation, physics::Scalar radius)
: physics::SphericalBody(physics_engine, false, radius)
, _formation(formation)
{
}

bool sim::PlanetaryBody::OnCollision(physics::Engine & engine, Body const & that_body) const
{
	dGeomID object_geom = that_body.GetGeomId();
	dGeomID planet_geom = GetGeomId();
	physics::IntersectionFunctor intersection_functor(engine, object_geom, planet_geom);
	
	that_body.OnDeferredCollisionWithPlanet(* this, intersection_functor);
	
	return true;
}

void sim::PlanetaryBody::OnDeferredCollisionWithBox(physics::Body const & body, physics::IntersectionFunctor & functor) const
{
	using namespace form::collision;

	physics::BoxBody const & box = static_cast<physics::BoxBody const &>(body);
	physics::Engine const & physics_engine = functor.GetEngine();
	form::Scene const & scene = physics_engine.GetScene();
	sim::Vector3 const & origin = scene.GetOrigin();
	
	// Get vital geometric information about the cuboid.
	Vector3 position = box.GetPosition();
	Vector3 dimensions = box.GetDimensions();
	Vector3 extents = dimensions * Scalar(.5);
	Matrix33 rotation = box.GetRotation();
	
	// Initialise the PointCloud.
	typedef Object<ConvexHull> Object;
	Object collision_object;
	
	// bounding sphere
	collision_object.bounding_sphere.center = form::SimToScene(position, origin);
	collision_object.bounding_sphere.radius = form::Scalar(Length(extents));
	
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
	
	form::Vector3 relative_formation_position(form::SimToScene(_formation.GetShape().center, origin));
	float min_box_edge = std::min(float(dimensions.x), std::min(float(dimensions.y), float(dimensions.z)));
	float min_parent_area = min_box_edge * formation_box_collision_detail_factor;
	
	ForEachCollision(* polyhedron, relative_formation_position, collision_object, origin, functor, min_parent_area);
}

void sim::PlanetaryBody::OnDeferredCollisionWithSphere(physics::Body const & body, physics::IntersectionFunctor & functor) const
{
	using namespace form::collision;

	physics::SphericalBody const & sphere = static_cast<physics::SphericalBody const &>(body);
	physics::Engine const & physics_engine = functor.GetEngine();
	form::Scene const & scene = physics_engine.GetScene();

	form::Polyhedron const * polyhedron = scene.GetPolyhedron(_formation);
	if (polyhedron == nullptr)
	{
		// This can happen if the PlanetaryBody has just been created 
		// and the corresponding OnAddFormation message hasn't been read yet.
		return;
	}
	
	typedef Object<form::Sphere3> Object;
	Object collision_object;
	sim::Vector3 const & origin = scene.GetOrigin();
	collision_object.bounding_sphere.center = form::SimToScene(sphere.GetPosition(), origin);
	collision_object.bounding_sphere.radius = float(sphere.GetRadius());
	
	collision_object.shape = collision_object.bounding_sphere;
	
	form::Vector3 relative_formation_position(form::SimToScene(_formation.GetShape().center, origin));
	float sphere_area(Area(collision_object.shape));
	float min_parent_area = sphere_area * formation_sphere_collision_detail_factor;

	ForEachCollision(* polyhedron, relative_formation_position, collision_object, origin, functor, min_parent_area);
}
