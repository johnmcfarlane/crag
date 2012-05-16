//
//  Engine.h
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
	class Engine;
	class EntitySet;
	
	
	// Engine - main object of simulation thread
	class Engine
	{
		OBJECT_SINGLETON(Engine);

	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Engine();
		~Engine();
		
		// message interface
		void OnQuit();
		
		template <typename OBJECT_TYPE, typename ... PARAMETERS>
		void CreateObject(Uid const & uid, PARAMETERS const & ... parameters)
		{
			smp::ObjectBaseInit<Engine> init = 
			{
				* this,
				uid
			};
			OBJECT_TYPE * object = new OBJECT_TYPE(init, parameters ...);
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
