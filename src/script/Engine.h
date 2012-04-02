//
//  Engine.h
//  crag
//
//  Created by John McFarlane on 1/19/11.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Script.h"

#include "smp/Daemon.h"

#include "core/Singleton.h"


namespace script
{
	typedef smp::Uid Uid;
	
	// script::Daemon type
	class Engine;
	typedef smp::Daemon<Engine> Daemon;
	
	// The scripting support is centered here.
	// When Run finished, the program is done.
	// TODO: Rename to physics::Engine (along with Simulation, Renderer and FormationManager)
	// and be careful to differentiate from the scheme/call-cc meaning).
	class Engine
	{
		OBJECT_SINGLETON(Engine);
		
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef std::queue<SDL_Event> EventQueue;
	public:
		typedef smp::Daemon<Engine> Daemon;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Engine();
		~Engine();

		// given a UID, returns the script associated with it
		Script * GetObject(Uid uid);
		
		// daemon messages
		void OnQuit();
		void OnEvent(SDL_Event const & event);
		
		template <typename SCRIPT_TYPE>
		void OnCreateObject(Uid const & uid)
		{
			SCRIPT_TYPE * script = new SCRIPT_TYPE ();
			script->SetUid(uid);
			OnAddObject(* script);
		}
		
		void OnAddObject(Script & entity);
		void OnRemoveObject(Uid const & uid);
		
		bool GetQuitFlag() const;
		void SetQuitFlag();
		
		Time GetTime() const;
		void SetTime(Time const & time);
		
		// thread entry point
		void Run(Daemon::MessageQueue & message_queue);
		
		void GetEvent(SDL_Event & event);
		
	private:
		bool HasFibersActive() const;
		
		bool ProcessTasks();
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		EventQueue _events;
		
		// Simulation time.
		Time _time;
		
		// Collection of all active scripts
		Script::List _scripts;
		
		bool _quit_flag;
	};
}
