//
//  ScriptThread.h
//  crag
//
//  Created by John McFarlane on 1/19/11.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Fiber.h"

#include "smp/Daemon.h"

#include "core/Singleton.h"


namespace script
{
	// script::Daemon type
	class ScriptThread;
	typedef smp::Daemon<ScriptThread> Daemon;
	
	// The scripting support is centered here.
	// When Run finished, the program is done.
	class ScriptThread
	{
		OBJECT_SINGLETON(ScriptThread);
		
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef std::queue<SDL_Event> EventQueue;
	public:
		typedef smp::Daemon<ScriptThread> Daemon;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		ScriptThread();
		~ScriptThread();
		
		// daemon messages
		void OnQuit();
		void OnEvent(SDL_Event const & event);
		
		bool GetQuitFlag() const;
		void SetQuitFlag();
		
		Time GetTime() const;
		void SetTime(Time const & time);
		
		// thread entry point
		void Run(Daemon::MessageQueue & message_queue);
		
		void GetEvent(SDL_Event & event);
		
		void Launch(Fiber & fiber_entry);
		
	private:
		bool HasFibersActive() const;
		
		bool ProcessTasks();
		bool StartTask();
		bool ContinueTask();
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		EventQueue _events;
		
		// Simulation time.
		Time _time;
		
		// Collection of all active fibers
		Fiber::List _fibers;
		
		// A new fiber to be launched next.
		Fiber * _unlaunched_fiber;
		
		bool _quit_flag;
	};
}
