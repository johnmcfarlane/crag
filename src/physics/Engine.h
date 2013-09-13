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
	class MeshSurround;
	
	// The physics singleton.
	class Engine
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		// store of all the contacts that occur in a Tick
		typedef std::vector<Contact> ContactVector;

		// maps from other geometry to planet tri-mesh local to other geometry
		typedef std::pair<CollisionHandle, CollisionHandle> MeshMapKey;
		typedef std::map<MeshMapKey, MeshSurround> MeshMap;

	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Engine();
		~Engine();
		
		form::Scene & GetScene();
		form::Scene const & GetScene() const;
		
		core::locality::Roster & GetPreTickRoster();
		core::locality::Roster & GetPostTickRoster();

		dBodyID CreateBody() const;
		CollisionHandle CreateBox(Vector3 const & dimensions) const;
		CollisionHandle CreateSphere(Scalar radius) const;
		CollisionHandle CreateRay(Scalar length) const;
		CollisionHandle CreateMesh(MeshData data) const;
		void DestroyShape(CollisionHandle shape);
		MeshSurround & GetMeshSurround(CollisionHandle geom1, CollisionHandle geom2);
		
		void Attach(Body const & body1, Body const & body2);
		
		void Tick(double delta_time, Ray3 const & camera_ray);
		
		void ToggleCollisions();
		
	private:
		void CreateCollisions();
		void CreateJoints();
		void DestroyJoints();
		void DestroyCollisions();
		static void OnNearCollisionCallback (void *data, CollisionHandle geom1, CollisionHandle geom2);
		
		// called on bodies which don't handling their own collision
		void OnUnhandledCollision(CollisionHandle geom1, CollisionHandle geom2);
		
	public:
		// Called once individual points of contact have been determined.
		void AddContact(ContactGeom const & contact_geom);
		
		template <typename ITERATOR>
		void AddContacts(ITERATOR begin, ITERATOR end)
		{
			auto count = end - begin;	// ITERATOR must be random access
			_contacts.reserve(_contacts.size() + count);
			std::for_each(begin, end, [=] (ContactGeom const & contact_geom) {
				AddContact(contact_geom);
			});
		}
		
	private:		
		// variables
		dWorldID world;
		dSpaceID space;
		dJointGroupID contact_joints;

		form::Scene & _formation_scene;
		
		// it seems that ODE keeps a hold of the contacts which are passed to it.
		ContactVector _contacts;
		dContact _contact;	// permanently stores common properties

		// list of objcets called at start/end of tick
		core::locality::Roster & _pre_tick_roster;
		core::locality::Roster & _post_tick_roster;
		
		// a secondary shape contains mesh data in the vacinity of other objects;
		// needs its lifetime managed somewhere central to physics simulation
		MeshMap _mesh_map;
	};
	
}
