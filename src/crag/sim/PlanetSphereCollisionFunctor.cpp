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

#include "form/Formation.h"
#include "form/scene/Polyhedron.h"

#include "form/node/ForEachNodeFace.h"
#include "form/node/RootNode.h"

#include "geom/SphereOps.h"


////////////////////////////////////////////////////////////////////////////////
// PlanetSphereCollisionFunctor members

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
}

sim::PlanetSphereCollisionFunctor::~PlanetSphereCollisionFunctor()
{
}

void sim::PlanetSphereCollisionFunctor::operator()(form::Point const & a, form::Point const & b, form::Point const & c, form::Vector3 const & normal)
{
	Vector3 sa(a.pos);
	Vector3 sb(b.pos);
	Vector3 sc(c.pos);
	
	// If the sphere is completely clear of the infinite plan of the triangle,
	Scalar planar_depth = DistanceToSurface<Scalar>(sa, sb, sc, Vector3(relative_sphere.center));
	if (planar_depth > sphere.radius)
	{
		// we're good. 
		return;
	}
	
	// One last check to see if we're 'in the zone' becuase the given triangle
	// might have been put together by ForEachNodeFace.
	if (! TouchesInfinitePyramid(sa, sb, sc))
	{
		return;
	}
	
	// The case where the sphere is completely within the surface. 
	if (planar_depth < sphere.radius)
	{
		Vector3 tri_center = (sa + sb + sc) / 3.;
		Vector3 to_tri = tri_center - relative_formation_center;
		Vector3 to_sphere = relative_sphere.center - relative_formation_center;
		Scalar dp = DotProduct(Normalized(to_tri), Normalized(to_sphere));
		if (dp < 0)
		{
			// yikes! colliding with a face on the opposite side of the formation
			return;
		}
		
		// TODO: Is the normal passed to AddFace negative? If so, lighting's still wrong. 
		OnContact(sphere.center, - normal, - planar_depth);
		return;
	}
	
	// Proper contact between triangle (not plane) and sphere.
	Scalar intersection_depth;
	if (Intersects(relative_sphere, sc, sb, sa, & intersection_depth))
	{
		//intersection_depth = planar_depth;
		// TODO: Triangle-line intersection. (Might already have this somewhere.)
		// because sphere.center is the wrong position!
		OnContact(sphere.center + Vector3(normal) * (sphere.radius - intersection_depth), normal, intersection_depth);
	}
}

void sim::PlanetSphereCollisionFunctor::SetSceneOrigin(sim::Vector3 const & in_scene_origin)
{
	scene_origin = in_scene_origin;
	InitRelativePositions();
}

void sim::PlanetSphereCollisionFunctor::operator()(form::Formation const & in_formation, form::Polyhedron const & in_model)
{
	if (& in_formation != & planet_formation)
	{
		return;
	}
	
	// Step 1: 
	form::RootNode const & root_node = in_model.root_node;
	form::Node const * children = root_node.GetChildren();
	if (children != nullptr)
	{
		GatherPoints(children[0]);
		GatherPoints(children[1]);
		GatherPoints(children[2]);
		GatherPoints(children[3]);
	}
}

void sim::PlanetSphereCollisionFunctor::InitRelativePositions()
{
	relative_sphere.center = form::SimToScene(sphere.center, scene_origin);
	relative_formation_center = form::SimToScene(planet_formation.position, scene_origin);
}

void sim::PlanetSphereCollisionFunctor::GatherPoints(form::Node const & node) 
{	
	// TODO: The same borders will be being tested many times. Should be able to cut down on them quite a bit.
	if (CanTraverse(node))
	{
		form::Node const * const children = node.GetChildren();
		if (children != nullptr)
		{
			form::Node const * child = children;
			form::Node const * const children_end = child + 4;
			do
			{
				GatherPoints(* child);
				++ child;
			}	while (child != children_end);
		}
		else 
		{
			ForEachNodeFace(node, * this);
		}
	}
}

// Returns true if the functor's sphere interescts with the infinite pyramic
// formed by projecting the planet's center through the given node's triangle.
bool sim::PlanetSphereCollisionFunctor::CanTraverse(form::Node const & node) const
{ 
	Vector3 a = node.GetCorner(0).pos;
	Vector3 b = node.GetCorner(1).pos;
	Vector3 c = node.GetCorner(2).pos;
	return TouchesInfinitePyramid(a, b, c);
}

bool sim::PlanetSphereCollisionFunctor::TouchesInfinitePyramid(Vector3 const & a, Vector3 const & b, Vector3 const & c) const
{
	return	Contains(relative_formation_center, a, c, relative_sphere) 
		&&	Contains(relative_formation_center, b, a, relative_sphere) 
		&&	Contains(relative_formation_center, c, b, relative_sphere);
}
