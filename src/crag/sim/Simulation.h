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

#include "smp/Actor.h"

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
	
	
	class Simulation : public smp::Actor<Simulation>
	{
		OBJECT_SINGLETON(Simulation);
	public:
		CONFIG_DECLARE_MEMBER (target_frame_seconds, sys::TimeType);

		Simulation();
		~Simulation();
		
		// Singleton
		static Simulation & Ref() { return ref(singleton); }
	
		// Message passing
		template <typename MESSAGE>
		static void SendMessage(MESSAGE const & message) 
		{ 
			Simulation & destination = Ref(); 
			smp::Actor<Simulation>::SendMessage(destination, message); 
		}

		// TODO: These should not be public. 
		void OnMessage(smp::TerminateMessage const & message);
		void OnMessage(AddEntityMessage const & message);
		void OnMessage(RemoveEntityMessage const & message);
		void OnMessage(gfx::RendererReadyMessage const & message);
		
	private:
		void Init();
		
	public:
		sys::TimeType GetTime() const;
		
		Universe & GetUniverse();

		physics::Engine & GetPhysicsEngine();		
		
	public:		
		bool HandleEvent(sys::Event const & event);

	private:
		void Run();
		void Tick();
		void UpdateRenderer() const;
		
		void PrintStats() const;
		void Capture();
		
		// Returns true until the program should ent.
		bool OnKeyPress(sys::KeyCode key_code);

		// Attributes
		bool quit_flag;
		bool paused;
		bool capture;
		int capture_frame;
		
		Universe * universe;
		physics::Engine * physics_engine;
		
		static Simulation * singleton;
	};
	
}
