/*
 *  Engine.h
 *  Crag
 *
 *  Created by John on 10/29/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "defs.h"


namespace physics
{
	
	class Engine
	{
		friend class Body;
		friend class SphericalBody;
		
	public:
		Engine();
		~Engine();

		void Tick(double delta_time);
		
		void ToggleCollisions();

	private:
		void CreateCollisions();
		void DestroyCollisions();
		static void OnNearCollisionCallback (void *data, dGeomID geom1, dGeomID geom2);
		
	public:
		void OnCollision(dGeomID geom1, dGeomID geom2);
		void OnContact(dContact const & contact);
		
	private:
		dBodyID CreateBody();

		dWorldID world;
		dSpaceID space;
		dJointGroupID contact_joints;
	};
	
}
