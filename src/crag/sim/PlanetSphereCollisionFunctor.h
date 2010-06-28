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

#include "form/FormationFunctor.h"
#include "form/defs.h"

#include "defs.h"

#include <ode/ode.h>

#include <vector>


namespace form 
{
	class Node;
}


namespace sim 
{
	
	////////////////////////////////////////////////////////////////////////////////
	// PlanetCollisionFunctor class

	class PlanetCollisionFunctor : public form::FormationFunctor 
	{
	public:
		PlanetCollisionFunctor(form::Formation const & in_planet_formation, dGeomID in_planet_geom, dGeomID in_object_geom);
		
	protected:
		void OnContact(Vector3 const & pos, Vector3 const & normal, Scalar depth);

		form::Formation const & planet_formation;
		dContact contact;
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	// PlanetSphereCollisionFunctor class
	
	class PlanetSphereCollisionFunctor : public PlanetCollisionFunctor
	{
	public:
		PlanetSphereCollisionFunctor(form::Formation const & in_planet_formation, 
									 dGeomID in_planet_geom, 
									 sim::Sphere3 const & in_sphere, 
									 dGeomID in_sphere_geom);
		~PlanetSphereCollisionFunctor();
		
		virtual void SetSceneOrigin(sim::Vector3 const & in_scene_origin);
		virtual void operator()(form::Formation const & in_formation, form::Model const & in_model);
		
	private:
		void InitRelativePositions();
		
		// Each point is a point on the surface of the formation which potentially collides with the sphere. 
		// When testing for collision, a segment is cast between the formation's center and the point
		// and it is this segment which is actually tested against the sphere. 
		// This way, if the sphere is wholly within the formation, they will still be collided. 
		struct Point 
		{
			Point(form::Vector3 const * init_pos);
			bool operator==(form::Vector3 const * rhs);
			
			// Initially, only this is stored. 
			// It's the point on the surface of the formation.
			form::Vector3 const * pos;
			
			// *pos + to_center == relative_formation_center
			Vector3 to_center;
			
			// The segment intersecs the sphere at (pos + to_center * t) and there are two values of t.
			Scalar average_t;
			Scalar range_t;
		};

		// Step 1: collect up all the points from triangles 
		// whose pyramids (when including the formation center)
		void GatherPoints(form::Node const & node);
		void AddPoint(form::Vector3 const * point);
		bool CanTraverse(form::Node const & node) const;
		bool IsInsideSurface(Vector3 const & j, Vector3 const & k, Vector3 const & l) const;		

		void TestPoints();
		bool TestPoint(Point & point);
		
		void SubmitPoints();
		
		void DebugDrawPoint(Point const & point) const;
		
		// Vars
		Vector3 scene_origin;
		Sphere3 sphere;
		Sphere3 relative_sphere;
		Vector3 relative_formation_center;
		
		// TODO: try std::set?
		typedef std::vector<Point> PointSet;
		PointSet points;
		
		static int max_num_points;
	};

}
