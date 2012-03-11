//
//  Simulation.h
//  crag
//
//  Created by John on 10/19/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

#include "smp/Daemon.h"

#include "core/ConfigEntry.h"
#include "core/Singleton.h"


namespace physics 
{
	class Engine;
}


namespace sim
{
	
	// forward declarations
	class Entity;
	class Simulation;
	class EntitySet;
	
	
	// Simulation - main object of simulation thread
	class Simulation
	{
		OBJECT_SINGLETON(Simulation);

	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Simulation();
		~Simulation();
		
		// message interface
		void OnQuit();
		
		template <typename OBJECT_TYPE>
		void OnCreateObject(Uid const & uid)
		{
			OBJECT_TYPE * object = new OBJECT_TYPE;
			object->SetUid(uid);
			object->Init(* this);
			OnAddObject(* object);
		}
		template <typename OBJECT_TYPE, typename INIT_DATA>
		void OnCreateObject(Uid const & uid, INIT_DATA const & init_data)
		{
			OBJECT_TYPE * object = new OBJECT_TYPE;
			object->SetUid(uid);
			object->Init(* this, init_data);
			OnAddObject(* object);
		}
		
		void OnAddObject(Entity & entity);
		void OnRemoveObject(Uid const & uid);
		void OnAttachEntities(Uid const & uid1, Uid const & uid2);
		
		void OnTogglePause();
		void OnToggleGravity();
		void OnToggleCollision();

		// accessors
		Time GetTime() const;
		Entity * GetObject(Uid uid);
		physics::Engine & GetPhysicsEngine();		
		
		// called be Daemon when simulation thread starts
		void Run(Daemon::MessageQueue & message_queue);
	private:
		void Tick();
		void UpdateRenderer() const;

		// call Tick on entities in the given object
		void TickEntities();
		
		////////////////////////////////////////////////////////////////////////////////
		// attributes
		
		bool quit_flag;
		bool paused;
		
		Time _time;

		EntitySet & _entity_set;
		physics::Engine & _physics_engine;
	};
	
}
