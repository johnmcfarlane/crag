//
//  Engine.h
//  crag
//
//  Created by John McFarlane on 1/19/11.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "ipc/Daemon.h"
#include "ipc/EngineBase.h"

namespace applet
{
	////////////////////////////////////////////////////////////////////////////////
	// forward-declarations
	
	DECLARE_CLASS_HANDLE(Applet);
	
	////////////////////////////////////////////////////////////////////////////////
	// definitions

	typedef ipc::Uid Uid;
	
	// applet::Daemon type
	class Engine;
	typedef ipc::Daemon<Engine> Daemon;
	
	// The applet scheduling is coordinated from here.
	// When Run finishes, the program is done.
	class Engine : public ipc::EngineBase <Engine, Applet>
	{
		////////////////////////////////////////////////////////////////////////////////
		// types

	public:
		typedef ipc::Daemon<Engine> Daemon;
		typedef ipc::EngineBase<Engine, Applet> super;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Engine();
		~Engine();

		// daemon messages
		void OnQuit();
		
		void SetQuitFlag();
		
		// thread entry point
		void Run(Daemon::MessageQueue & message_queue);
		
	private:
		bool HasFibersActive() const;
		
		core::Time ProcessTasks();
		core::Time ProcessTask(Applet & applet);
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		bool _quit_flag;
	};
}
