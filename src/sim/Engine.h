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
#include "smp/EngineBase.h"

#include "geom/origin.h"

#include "core/Singleton.h"

namespace core
{
	namespace locality
	{
		class Roster;
	}
}

namespace physics 
{
	class Engine;
}

namespace form
{
	class Formation;
}

namespace sim
{
	
	// forward declarations
	class Entity;
	
	// Engine - main object of simulation thread
	class Engine : public smp::EngineBase<Engine, Entity>
	{
		OBJECT_SINGLETON(Engine);

	public:
		typedef smp::EngineBase<Engine, Entity> super;
		typedef smp::Daemon<Engine> Daemon;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Engine();
		~Engine();
		
		// message interface
		void OnQuit();
		
		virtual void OnAddObject(super::Object & entity) override final;
		void OnAttachEntities(Uid uid1, Uid uid2);
		
		void AddFormation(form::Formation& formation);
		void RemoveFormation(form::Formation& formation);
		
		void SetCamera(geom::rel::Ray3 const & camera);
		geom::rel::Ray3 const & GetCamera() const;

		void OnSetOrigin(geom::abs::Vector3 const & origin);
		
		void OnTogglePause();
		void OnToggleGravity();
		void OnToggleCollision();

		// accessors
		core::Time GetTime() const;
		physics::Engine & GetPhysicsEngine();		
		core::locality::Roster & GetTickRoster();

		// called be Daemon when simulation thread starts
		void Run(Daemon::MessageQueue & message_queue);
	private:
		void Tick();
		void UpdateRenderer() const;

		// call Tick on entities in the given object
		void PurgeEntities();
		
		////////////////////////////////////////////////////////////////////////////////
		// attributes
		
		bool quit_flag;
		bool paused;
		
		core::Time _time;

		Ray3 _camera;
		physics::Engine & _physics_engine;

		core::locality::Roster & _tick_roster;
	};
	
}
