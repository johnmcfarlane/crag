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
	
	class Simulation;
	class Universe;
	
	struct AddEntityMessage;
	struct RemoveEntityMessage;
	struct TogglePauseMessage { };
	struct ToggleCaptureMessage { };
	struct ToggleGravityMessage { };
	struct ToggleCollisionMessage { };
	
	
	class Simulation
	{
		OBJECT_SINGLETON(Simulation);

	public:
		typedef smp::Daemon<Simulation> Daemon;

		CONFIG_DECLARE_MEMBER (target_frame_seconds, sys::TimeType);

		Simulation();
		~Simulation();
		
		void OnMessage(smp::TerminateMessage const & message);
		void OnMessage(AddEntityMessage const & message);
		void OnMessage(RemoveEntityMessage const & message);
		void OnMessage(TogglePauseMessage const & message);
		void OnMessage(ToggleGravityMessage const & message);
		void OnMessage(ToggleCollisionMessage const & message);
		void OnMessage(gfx::RendererReadyMessage const & message);
		
	private:
		void Init();
		
	public:
		sys::TimeType GetTime() const;
		
		Universe & GetUniverse();

		physics::Engine & GetPhysicsEngine();		
		
		void Run(Daemon::MessageQueue & message_queue);
	private:
		void Tick();
		void UpdateRenderer() const;
		
		// Attributes
		bool quit_flag;
		bool paused;
		
		Universe * universe;
		physics::Engine * physics_engine;
	};
	
}