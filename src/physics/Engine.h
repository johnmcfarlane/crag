//
//  Engine.h
//  crag
//
//  Created by John on 10/29/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

#include "smp/vector.h"

#include <ode/contact.h>

namespace core
{
	namespace locality
	{
		class Roster;
	}
}

namespace form
{
	class Scene;
}

namespace physics
{
	// forward-declarations
	class Body;
	
	// The physics singleton.
	class Engine
	{
		// types
		typedef smp::vector<Contact> ContactVector;
	public:
		
		// functions
		Engine();
		~Engine();
		
		form::Scene & GetScene();
		form::Scene const & GetScene() const;
		
		core::locality::Roster & GetRoster();

		dBodyID CreateBody() const;
		dGeomID CreateBox(Vector3 const & dimensions) const;
		dGeomID CreateSphere(Scalar radius) const;
		dGeomID CreateRay(Scalar length) const;
		
		void Attach(Body const & body1, Body const & body2);
		
		void Tick(double delta_time, Ray3 const & camera_ray);
		
		void ToggleCollisions();
		
	private:
		void CreateCollisions();
		void CreateJoints();
		void DestroyJoints();
		void DestroyCollisions();
		static void OnNearCollisionCallback (void *data, CollisionHandle geom1, CollisionHandle geom2);
		
		// Called by bodies which don't handling their own.
		void OnUnhandledCollision(CollisionHandle geom1, CollisionHandle geom2);
		
	public:
		// Called once individual points of contact have been determined.
		void OnContact(Contact const & contact);
		
	private:		
		// variables
		dWorldID world;
		dSpaceID space;
		dJointGroupID contact_joints;

		form::Scene & _formation_scene;
		
		// it seems that ODE keeps a hold of the contacts which are passed to it.
		ContactVector _contacts;

		// list of objcets called at end of tick
		core::locality::Roster & _tick_roster;
	};
	
}
