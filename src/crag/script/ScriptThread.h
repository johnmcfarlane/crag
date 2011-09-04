/*
 *  ScriptThread.h
 *  crag
 *
 *  Created by John McFarlane on 1/19/11.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "smp/Daemon.h"

#include "sys/App.h"

#include "core/Singleton.h"


namespace script
{
	
	struct EventMessage
	{
		sys::Event event;
	};
	
	// The scripting support is centered here.
	// When Run finished, the program is done.
	class ScriptThread
	{
		OBJECT_SINGLETON(ScriptThread);
		
		// types
		typedef std::queue<PyObject *> EventQueue;
		
	public:
		typedef smp::Daemon<ScriptThread> Daemon;

		ScriptThread();
		~ScriptThread();
		
		void OnMessage(EventMessage const & message);
		void OnMessage(smp::TerminateMessage const & message);

		// thread entry point
		void Run(Daemon::MessageQueue & message_queue);
		
		PyObject * PollEvent();
		
	private:		
		static bool RedirectPythonOutput(char const * filename);

		// variables
		static char const * _source_filename;
		FILE * _source_file;
		Daemon::MessageQueue * _message_queue;
		EventQueue _events;
	};
	
}
