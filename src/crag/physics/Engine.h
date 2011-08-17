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

#include "smp/Mutex.h"
#include "smp/scheduler.h"

#include "core/ring_buffer.h"


namespace physics
{
	class Body;
	
	class Engine
	{
		friend class Body;
		friend class SphericalBody;
		
	public:
		// types
		typedef smp::scheduler::Batch DeferredCollisionBuffer;
		
		// functions
		Engine();
		~Engine();

		void Tick(double delta_time);
		
		void ToggleCollisions();

	private:
		void CreateCollisions();
		void ProcessDeferredCollisions();
		void DestroyCollisions();
		static void OnNearCollisionCallback (void *data, dGeomID geom1, dGeomID geom2);
		
	public:
		// Called by bodies which don't handling their own.
		void OnUnhandledCollision(dGeomID geom1, dGeomID geom2);
		
		// Called by bodies whose collision may be costly and can be parallelized.
		template <typename FUNCTOR>
		void DeferCollision(FUNCTOR & collision_functor)
		{
			while (! _deferred_collisions.push_back(collision_functor))
			{
				size_t capacity = _deferred_collisions.capacity();
				if (! _deferred_collisions.reserve(capacity * 2))
				{
					Assert(false);
				}
			}
		}
		
		// Called once individual points of contact have been determined.
		void OnContact(dContact const & contact);
		
	private:
		dBodyID CreateBody();

		// variables
		dWorldID world;
		dSpaceID space;
		dJointGroupID contact_joints;
		
		smp::Mutex _mutex;
		DeferredCollisionBuffer _deferred_collisions;
	};
	
}
