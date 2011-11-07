/*
 *  Simulation.h
 *  Crag
 *
 *  Created by John on 10/19/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "defs.h"

#include "smp/Daemon.h"

#include "core/ConfigEntry.h"
#include "core/Singleton.h"

#include "sys/App.h"


namespace physics 
{
	class Engine;
}

namespace gfx
{
	struct RendererReadyMessage;
}


namespace sim
{
	
	// forward declarations
	class Entity;
	class Simulation;
	class EntitySet;
	

	// daemon type
	typedef smp::Daemon<Simulation> Daemon;
	
	
	// Simulation - main object of simulation thread
	class Simulation
	{
		OBJECT_SINGLETON(Simulation);

	public:
		CONFIG_DECLARE_MEMBER (target_frame_seconds, Time);
		CONFIG_DECLARE_MEMBER (apply_gravity, bool);

		Simulation();
		~Simulation();
		
		// message interface
		void OnQuit();
		void OnAddEntity(Entity * const & entity, PyObject * const & args);
		void OnRemoveEntity(Entity * const & entity);
		void OnTogglePause();
		void OnToggleGravity();
		void OnToggleCollision();
		
	private:
		void Init();
		
	public:
		Time GetTime() const;
		
		EntitySet & GetEntities();

		physics::Engine & GetPhysicsEngine();		
		
		void Run(Daemon::MessageQueue & message_queue);
	private:
		void Tick();
		void UpdateRenderer() const;
		
		void TickEntities();
		void UpdateModels() const;
		
		// Attributes
		bool quit_flag;
		bool paused;
		
		Time _time;

		EntitySet & _entity_set;
		physics::Engine & _physics_engine;
	};
	
}
