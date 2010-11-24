/*
 *  PlanetSphereCollisionFunctor.h
 *  crag
 *
 *  Created by John on 6/24/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "PlanetCollisionFunctor.h"

#include "form/defs.h"


namespace form 
{
	class Node;
	class Point;
}


namespace sim 
{
	
	////////////////////////////////////////////////////////////////////////////////
	// PlanetSphereCollisionFunctor class
	//
	// Deals with Planet-Sphere collision.
	
	class PlanetSphereCollisionFunctor : public PlanetCollisionFunctor
	{
	public:
		PlanetSphereCollisionFunctor(form::Formation const & in_planet_formation, 
									 dGeomID in_planet_geom, 
									 sim::Sphere3 const & in_sphere, 
									 dGeomID in_sphere_geom);
		~PlanetSphereCollisionFunctor();
		
		// Slightly hacky - this class is two functors: a FormationFunctor and a ForEachNodeFace functor.
		void AddFace(form::Point const & a, form::Point const & b, form::Point const & c, form::Vector3 const & normal);
		
	private:
		virtual void SetSceneOrigin(sim::Vector3 const & in_scene_origin);
		virtual void operator()(form::Formation const & in_formation, form::Polyhedron const & in_model);
	
		void InitRelativePositions();

		void GatherPoints(form::Node const & node);
		bool CanTraverse(form::Node const & node) const;
		bool TouchesInfinitePyramid(Vector3 const & a, Vector3 const & b, Vector3 const & c) const;
		
		//void DebugDrawPoint(Point const & point) const;
		
		// Vars
		Vector3 scene_origin;
		Sphere3 sphere;
		Sphere3 relative_sphere;
		Vector3 relative_formation_center;
	};

}
