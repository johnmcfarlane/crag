//
//  Engine.h
//  crag
//
//  Created by John McFarlane on 1/19/11.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "ScriptBase.h"

#include "smp/Daemon.h"

#include "core/Singleton.h"


namespace script
{
	////////////////////////////////////////////////////////////////////////////////
	// forward-declarations
	
	template <typename FUNCTOR> class Script;
	
	////////////////////////////////////////////////////////////////////////////////
	// definitions

	typedef smp::Uid Uid;
	
	// script::Daemon type
	class Engine;
	typedef smp::Daemon<Engine> Daemon;
	
	// The scripting support is centered here.
	// When Run finished, the program is done.
	// Note: unrelated to Scheme engine:
	// [http://en.wikipedia.org/wiki/Engine_(computer_science)]
	class Engine
	{
		OBJECT_SINGLETON(Engine);
		
		////////////////////////////////////////////////////////////////////////////////
		// types

	public:
		typedef smp::Daemon<Engine> Daemon;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Engine();
		~Engine();

		// given a UID, returns the script associated with it
		ScriptBase * GetObject(Uid uid);
		
		// daemon messages
		template <typename FUNCTOR = void (*) (FiberInterface & fiber)>
		void Launch(FUNCTOR & functor)
		{
			ScriptBase * object = new Script<FUNCTOR> (functor);
			OnAddObject(object);
		}
		
		void OnQuit();
		
		template <typename SCRIPT_TYPE>
		void OnCreateObject(Uid const & uid)
		{
			SCRIPT_TYPE * script = new SCRIPT_TYPE ();
			script->SetUid(uid);
			OnAddObject(script);
		}
		
		void OnAddObject(ScriptBase * const & entity);
		void OnRemoveObject(Uid const & uid);
		
		void SetQuitFlag();
		
		// thread entry point
		void Run(Daemon::MessageQueue & message_queue);
		
	private:
		bool HasFibersActive() const;
		
		bool ProcessTasks();
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		// sim::Engine time.
		Time _time;
		
		// Collection of all active scripts
		ScriptBase::List _scripts;
		
		bool _quit_flag;
	};
}
