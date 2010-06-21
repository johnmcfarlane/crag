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


namespace sim 
{
	class Entity;
	class PlanetaryBody;
}

namespace physics
{
	class Body;
	class SphericalBody;
	
	class Singleton : public core::Singleton<Singleton>
	{
	public:
		Singleton();
		~Singleton();

		SphericalBody * CreateSphericalBody(sim::Entity & entity, float radius, bool movable);
		sim::PlanetaryBody * CreatePlanetaryBody(sim::Entity & entity, float radius, bool movable);
		void DestroyBody(Body & body);

		void Tick(double delta_time);

	private:
		void CreateCollisions();
		void DestroyCollisions();
		static void OnNearCollisionCallback (void *data, dGeomID geom1, dGeomID geom2);
		
	public:
		void OnCollision(dGeomID geom1, dGeomID geom2);
		
	private:
		void OnContact(dContact * contact, dGeomID geom1, dGeomID geom2);
		dBodyID CreateBody(sim::Entity & entity);

		dWorldID world;
		dSpaceID space;
		dJointGroupID contact_joints;
	};
	
}
