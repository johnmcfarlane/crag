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

#include "form/ForEachNodeFace.h"
#include "form/Formation.h"
#include "form/Model.h"
#include "form/RootNode.h"

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
	
	// Step 1: 
	form::RootNode const & root_node = in_model.root_node;
	form::Node const * children = root_node.children;
	if (children != nullptr)
	{
		GatherPoints(children[0]);
		GatherPoints(children[1]);
		GatherPoints(children[2]);
		GatherPoints(children[3]);
	}
}

void sim::PlanetSphereCollisionFunctor::AddFace(form::Point const & a, form::Point const & b, form::Point const & c, form::Vector3 const & normal)
{
	Vector3 sa(a);
	Vector3 sb(b);
	Vector3 sc(c);
	
	// If the sphere is completely clear of the infinite plan of the triangle,
	Scalar planar_depth = DistanceToSurface<Scalar>(sa, sb, sc, Vector3(relative_sphere.center));
	if (planar_depth < - sphere.radius)
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
	if (planar_depth > sphere.radius)
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
		
		// deep penetration :o
		OnContact(sphere.center, normal, planar_depth);
		return;
	}
	
	// Proper contact between triangle (not plane) and sphere.
	Scalar intersection_depth;
	if (Intersects(relative_sphere, sa, sb, sc, & intersection_depth))
	{
		//intersection_depth = planar_depth;
		// TODO: Triangle-line intersection. (Might already have this somewhere.)
		// because sphere.center is the wrong position!
		OnContact(sphere.center + Vector3(normal) * (sphere.radius - intersection_depth), normal, intersection_depth);
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
		if (node.children != nullptr)
		{
			for (int i = 0; i < 4; ++ i)
			{
				form::Node const & child = node.children[i];
				GatherPoints(child);
			}
		}
		else 
		{
			ForEachNodeFace(node, * this);
		}
	}
}

// TODO: Better name.
bool sim::PlanetSphereCollisionFunctor::CanTraverse(form::Node const & node) const
{ 
	Vector3 a = node.GetCorner(0);
	Vector3 b = node.GetCorner(1);
	Vector3 c = node.GetCorner(2);
	return TouchesInfinitePyramid(a, b, c);
}

bool sim::PlanetSphereCollisionFunctor::TouchesInfinitePyramid(Vector3 const & a, Vector3 const & b, Vector3 const & c) const
{
	Vector3f g;
	return	Contains(relative_formation_center, a, b, relative_sphere) 
		&&	Contains(relative_formation_center, b, c, relative_sphere) 
		&&	Contains(relative_formation_center, c, a, relative_sphere);
}
