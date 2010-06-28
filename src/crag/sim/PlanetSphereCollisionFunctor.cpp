/*
 *  PlanetSphereCollisionFunctor.cpp
 *  crag
 *
 *  Created by John on 6/24/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "PlanetSphereCollisionFunctor.h"

#include "physics/Singleton.h"

#include "form/Formation.h"
#include "form/Model.h"
#include "form/RootNode.h"

#include "gfx/Debug.h"

#include "core/SphereOps.h"


////////////////////////////////////////////////////////////////////////////////
// PlanetCollisionFunctor members

sim::PlanetCollisionFunctor::PlanetCollisionFunctor(form::Formation const & in_planet_formation, dGeomID in_planet_geom, dGeomID in_object_geom)
: planet_formation(in_planet_formation)
{
	contact.geom.g1 = in_planet_geom;
	contact.geom.g2 = in_object_geom;
}

void sim::PlanetCollisionFunctor::OnContact(Vector3 const & pos, Vector3 const & normal, Scalar depth)
{
	contact.surface.mode = dContactBounce /*| dContactSoftCFM*/;
	contact.surface.mu = 1;				// used (by default)
	contact.surface.bounce = .5f;		// used
	contact.surface.bounce_vel = .1f;	// used
	contact.geom.pos[0] = pos.x;
	contact.geom.pos[1] = pos.y;
	contact.geom.pos[2] = pos.z;
	contact.geom.normal[0] = normal.x;
	contact.geom.normal[1] = normal.y;
	contact.geom.normal[2] = normal.z;
	contact.geom.depth = depth;
	
	physics::Singleton & singleton = physics::Singleton::Get();
	singleton.OnContact(contact, contact.geom.g1, contact.geom.g2);
}


////////////////////////////////////////////////////////////////////////////////
// PlanetSphereCollisionFunctor members

sim::PlanetSphereCollisionFunctor::Point::Point(form::Vector3 const * init_pos) 
: pos(init_pos) 
{ 
}

bool sim::PlanetSphereCollisionFunctor::Point::operator==(form::Vector3 const * rhs)
{
	return pos == rhs;
}


// Slightly hacky optimization:
// This remembers the maximum point array size and reserves this much next time.
int sim::PlanetSphereCollisionFunctor::max_num_points = 0;


sim::PlanetSphereCollisionFunctor::PlanetSphereCollisionFunctor(form::Formation const & in_planet_formation, 
																dGeomID in_planet_geom, 
																sim::Sphere3 const & in_sphere, 
																dGeomID in_sphere_geom)
: PlanetCollisionFunctor(in_planet_formation, in_planet_geom, in_sphere_geom)
, scene_origin(sim::Vector3::Zero())
, sphere(in_sphere)
{
	relative_sphere.radius = in_sphere.radius;
	InitRelativePositions();
	
	points.reserve(max_num_points);
}

sim::PlanetSphereCollisionFunctor::~PlanetSphereCollisionFunctor()
{
}

void sim::PlanetSphereCollisionFunctor::SetSceneOrigin(sim::Vector3 const & in_scene_origin)
{
	scene_origin = in_scene_origin;
	InitRelativePositions();
}

void sim::PlanetSphereCollisionFunctor::operator()(form::Formation const & in_formation, form::Model const & in_model)
{
	if (& in_formation != & planet_formation)
	{
		return;
	}
	
	form::RootNode const & root_node = in_model.root_node;
	GatherPoints(root_node);
	TestPoints();
	SubmitPoints();
}

void sim::PlanetSphereCollisionFunctor::InitRelativePositions()
{
	relative_sphere.center = form::SimToScene(sphere.center, scene_origin);
	relative_formation_center = form::SimToScene(planet_formation.position, scene_origin);
}

void sim::PlanetSphereCollisionFunctor::GatherPoints(form::Node const & node) 
{	
	form::Node::Triplet const * t = node.triple;
	form::Node::Triplet const * const end = t + 3;
	while (t != end)
	{
		form::Point const * mid_point = t->mid_point;
		if (mid_point != nullptr)
		{
			AddPoint(mid_point);
		}
		
		++ t;
	}
	
	if (node.children != nullptr)
	{
		for (int i = 0; i < 4; ++ i)
		{
			form::Node const & child = node.children[i];
			
			// TODO: The same borders will be being tested many times. Should be able to cut down on them quite a bit.
			if (CanTraverse(child))
			{
				GatherPoints(child);
			}
		}
	}
}

void sim::PlanetSphereCollisionFunctor::AddPoint(form::Vector3 const * point)
{
	PointSet::iterator i = find(points.begin(), points.end(), point);
	if (i == points.end())
	{
		points.push_back(point);
	}
}

// TODO: Better name.
bool sim::PlanetSphereCollisionFunctor::CanTraverse(form::Node const & node) const
{ 
	Vector3 a = node.GetCorner(0);
	Vector3 b = node.GetCorner(1);
	Vector3 c = node.GetCorner(2);
	
	if ( ! (IsInsideSurface(relative_formation_center, a, b) 
			&& IsInsideSurface(relative_formation_center, b, c) 
			&& IsInsideSurface(relative_formation_center, c, a)))
	{
		return false;
	}
	
	return true;
}

bool sim::PlanetSphereCollisionFunctor::IsInsideSurface(Vector3 const & j, Vector3 const & k, Vector3 const & l) const
{
	Vector3f normal = TriangleNormal(j, k, l);
	Normalize(normal);
	
	return DotProduct(normal, relative_sphere.center - k) < sphere.radius;
}

void sim::PlanetSphereCollisionFunctor::TestPoints()
{
	// Before we start reducing the number of points, remember the maximum for next time.
	int num_points = points.size();
	if (num_points > max_num_points)
	{
		max_num_points = num_points;
	}

	// For all points gathered.
	for (PointSet::iterator i = points.begin(); i != points.end(); )
	{
		Point & point = * i;
		
		// Test collision on point and if it fails,
		if (! TestPoint(point))
		{
			// removed it (unordered).
			point.pos = points.back().pos;
			points.pop_back();
		}
		else 
		{
			++ i;
		}
	}
}

bool sim::PlanetSphereCollisionFunctor::TestPoint(Point & point)
{
	Vector3 point_pos = * point.pos;
	Vector3 delta = relative_formation_center - point_pos;
	Scalar t1, t2;

	DebugDrawPoint(point);
	
	if (! GetIntersection(relative_sphere, point_pos, delta, t1, t2))
	{
		return false;
	}
	
	if (t2 <= 0 || t1 >= 1)
	{
		return false;
	}

	// Calmp the collision numbers to the range that's within the line.
	if (t1 < 0)
	{
		t1 = 0;
	}
	if (t2 > 1)
	{
		t2 = 1;
	}

	point.to_center = delta;
	
	point.range_t = t2 - t1;
	Assert(point.range_t > 0);
	
	point.average_t = (t1 + t2) * .5;
	
	return true;
}

void sim::PlanetSphereCollisionFunctor::SubmitPoints() 
{
	Scalar inverse_num_points = 1.0 / points.size();
	
	for (PointSet::const_iterator i = points.begin(); i != points.end(); ++ i)
	{
		Point const & point = * i;

		// The depth of the collision.
		float l = Length(point.to_center);
		Scalar depth = point.range_t * l;
		Assert(depth <= sphere.radius * 2);
		
		// The center of the collision.
		Vector3 relative_middle = Vector3(* point.pos) + point.to_center * point.average_t;
		
		Vector3 contact_point = form::SceneToSim(relative_middle, scene_origin);
		
		Vector3 normal = relative_sphere.center - contact_point;
		Normalize(normal);
		
		OnContact(contact_point, normal, depth * inverse_num_points);
	}
}

void sim::PlanetSphereCollisionFunctor::DebugDrawPoint(Point const & point) const
{
	gfx::Debug::AddLine(form::SceneToSim(Vector3(* point.pos), scene_origin), 
						form::SceneToSim(relative_formation_center, 
										 scene_origin));
}
