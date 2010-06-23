/*
 *  Singleton.h
 *  Crag
 *
 *  Created by John on 10/29/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "defs.h"

#include "core/Singleton.h"


namespace physics
{
	class Singleton : public core::Singleton<Singleton>
	{
		friend class Body;
		friend class SphericalBody;
		
	public:
		Singleton();
		~Singleton();

		void Tick(double delta_time);

	private:
		void CreateCollisions();
		void DestroyCollisions();
		static void OnNearCollisionCallback (void *data, dGeomID geom1, dGeomID geom2);
		
	public:
		void OnCollision(dGeomID geom1, dGeomID geom2);
		
	private:
		void OnContact(dContact * contact, dGeomID geom1, dGeomID geom2);
		dBodyID CreateBody();

		dWorldID world;
		dSpaceID space;
		dJointGroupID contact_joints;
	};
	
}
