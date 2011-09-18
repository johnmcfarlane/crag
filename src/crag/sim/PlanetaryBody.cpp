/*
 *  FormationBody.cpp
 *  crag
 *
 *  Created by John on 6/20/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "PlanetaryBody.h"

#include "defs.h"
#include "Simulation.h"

#include "physics/BoxBody.h"
#include "physics/Engine.h"
#include "physics/IntersectionFunctor.h"

#include "form/Formation.h"
#include "form/FormationManager.h"
#include "form/node/NodeBuffer.h"
#include "form/scene/ForEachIntersection.h"


namespace
{
	
	////////////////////////////////////////////////////////////////////////////////
	// config constants
	
	CONFIG_DEFINE (formation_sphere_collision_detail_factor, float, 10.f);
	CONFIG_DEFINE (formation_box_collision_detail_factor, float, 5);
	
	
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
			Assert(unit_index == 0);
			
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
	physics::IntersectionFunctor intersection_functor(object_geom, planet_geom);
	
	DeferredIntersectionFunctor deferred_functor(that_body, * this, intersection_functor);	
	engine.DeferCollision(deferred_functor);
	
	return true;
}

void sim::PlanetaryBody::OnDeferredCollisionWithBox(physics::Body const & body, physics::IntersectionFunctor & functor) const
{
	physics::BoxBody const & box = static_cast<physics::BoxBody const &>(body);
	form::FormationManager const & formation_manager = form::FormationManager::Daemon::Ref();
	form::Scene const & scene = formation_manager.OnTreeQuery();
	sim::Vector3 const & origin = scene.GetOrigin();
	
	// Get vital geometric information about the cuboid.
	Vector3 dimensions = box.GetDimensions();
	Matrix4 rotation;
	box.GetRotation(rotation);
	
	// Initialise the PointCloud.
	form::PointCloud shape;
	
	// bounding sphere
	shape.sphere.center = form::SimToScene(box.GetPosition(), origin);
	shape.sphere.radius = form::Scalar(Length(dimensions));
	
	// points
	int const num_corners = 8;
	shape.points.resize(num_corners);
	for (int corner_index = 0; corner_index != num_corners; ++ corner_index)
	{
		sim::Vector4 corner;
		corner.x = ((corner_index & 1) ? dimensions.x : - dimensions.x) * .5;
		corner.y = ((corner_index & 2) ? dimensions.y : - dimensions.y) * .5;
		corner.z = ((corner_index & 4) ? dimensions.z : - dimensions.z) * .5;
		corner.w = 0;
		sim::Vector4 rotated_corner = rotation * corner;
		shape.points[corner_index].x = rotated_corner.x + shape.sphere.center.x;
		shape.points[corner_index].y = rotated_corner.y + shape.sphere.center.y;
		shape.points[corner_index].z = rotated_corner.z + shape.sphere.center.z;
	}
	
	// TODO: Try and move as much of this as possible into the ForEachIntersection fn.
	form::Polyhedron const * polyhedron = scene.GetPolyhedron(_formation);
	if (polyhedron == nullptr)
	{
		Assert(false);
		return;
	}
	
	form::Vector3 relative_formation_position(form::SimToScene(_formation.position, origin));
	form::NodeBuffer const & node_buffer = scene.GetNodeBuffer();
	float min_parent_score = node_buffer.GetMinParentScore() * formation_box_collision_detail_factor;
	
	form::ForEachIntersection(* polyhedron, relative_formation_position, shape, origin, functor, min_parent_score);
}

void sim::PlanetaryBody::OnDeferredCollisionWithSphere(physics::Body const & body, physics::IntersectionFunctor & functor) const
{
	physics::SphericalBody const & sphere = static_cast<physics::SphericalBody const &>(body);
	form::FormationManager const & formation_manager = form::FormationManager::Daemon::Ref();
	form::Scene const & scene = formation_manager.OnTreeQuery();
	sim::Vector3 const & origin = scene.GetOrigin();
	
	form::Sphere3 relative_sphere(form::SimToScene(sphere.GetPosition(), origin), form::Scalar(sphere.GetRadius()));
	
	form::Polyhedron const * polyhedron = scene.GetPolyhedron(_formation);
	if (polyhedron == nullptr)
	{
		Assert(false);
		return;
	}
	
	form::Vector3 relative_formation_position(form::SimToScene(_formation.position, origin));
	form::NodeBuffer const & node_buffer = scene.GetNodeBuffer();
	float min_parent_score = node_buffer.GetMinParentScore() * formation_sphere_collision_detail_factor;
	
	form::ForEachIntersection(* polyhedron, relative_formation_position, relative_sphere, origin, functor, min_parent_score);
}
