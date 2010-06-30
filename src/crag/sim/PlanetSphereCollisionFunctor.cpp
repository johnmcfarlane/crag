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
	
	Scalar planar_depth = sphere.radius - GetDistanceToSurface(sc, sb, sa);
	if (planar_depth < 0)
	{
		return;
	}
	
	Scalar intersection_depth;
	if (Intersects(relative_sphere, sa, sb, sc, & intersection_depth))
	{
		// TODO: Triangle-line intersection. (Might already have this somewhere.)
		// because sphere.center is the wrong position!
		OnContact(sphere.center - Vector3(normal) * intersection_depth, normal, intersection_depth);
		return;
	}
	
	if (planar_depth > sphere.radius)
	{
		Vector3 tri_center = (sa + sb + sc) / 3.;
		Vector3 to_tri = tri_center - relative_formation_center;
		Vector3 to_sphere = relative_sphere.center - relative_formation_center;
		if (DotProduct(to_tri, to_sphere) < 0)
		{
			// yikes! colliding with a face on the opposite side of the formation
			return;
		}
		
		// deep penetration :o
		OnContact(sphere.center, normal, planar_depth);
		return;
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
	return GetDistanceToSurface(j, k, l) < sphere.radius;
}

float sim::PlanetSphereCollisionFunctor::GetDistanceToSurface(Vector3 const & j, Vector3 const & k, Vector3 const & l) const
{
	Vector3f normal = TriangleNormal(j, k, l);
	Normalize(normal);
	
	return DotProduct(normal, relative_sphere.center - k);
}
