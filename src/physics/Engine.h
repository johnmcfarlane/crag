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

namespace crag
{
	namespace core
	{
		class Roster;
	}
}

namespace form
{
	class RayCastResult;
}

namespace physics
{
	// forward-declarations
	class Body;
	
	// The physics singleton.
	class Engine
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		// store of all the contacts that occur in a Tick
		typedef std::vector<Contact> ContactVector;

	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		OBJECT_NO_COPY(Engine);

		Engine();
		~Engine();
		
		crag::core::Roster & GetPreTickRoster();
		crag::core::Roster & GetPostTickRoster();

		dBodyID CreateBody() const;
		CollisionHandle CreateBox(Vector3 const & dimensions) const;
		CollisionHandle CreateSphere(Scalar radius) const;
		CollisionHandle CreateCylinder(Scalar radius, Scalar length) const;
		CollisionHandle CreateRay(Scalar length) const;
		CollisionHandle CreateMesh(MeshData data) const;
		void DestroyShape(CollisionHandle shape);
		
		void Attach(Body const & body1, Body const & body2);
		
		void Tick(double delta_time);
		
		// use sparingly
		form::RayCastResult CastRay(Ray3 const & ray, Scalar length, Body const * exception = nullptr);
		void Collide(Sphere3 const & sphere, ContactFunction & contact_function);
		
		void ToggleCollisions();
	private:
		void CreateCollisions();
		void CreateJoints();
		void DestroyJoints();
		void DestroyCollisions();
		static void OnNearCollisionCallback (void *data, CollisionHandle geom1, CollisionHandle geom2);
		
		// called on bodies which don't handling their own collision
		void OnUnhandledCollision(CollisionHandle geom1, CollisionHandle geom2);

		void AddContacts(ContactGeom const * begin, ContactGeom const * end);

	private:		
		// variables
		dWorldID world;
		dSpaceID space;
		dJointGroupID contact_joints;

		// it seems that ODE keeps a hold of the contacts which are passed to it.
		ContactVector _contacts;
		dContact _contact;	// permanently stores common properties

		// list of objcets called at start/end of tick
		std::unique_ptr<crag::core::Roster> _pre_tick_roster;
		std::unique_ptr<crag::core::Roster> _post_tick_roster;
	};
	
}
