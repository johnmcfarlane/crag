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
#include "core/Singleton.h"


namespace script
{
	
	// The scripting support is centered here.
	// When Run finished, the program is done.
	class ScriptThread
	{
		OBJECT_SINGLETON(ScriptThread);
		
	public:
		typedef smp::Daemon<ScriptThread, false> Daemon;

		ScriptThread();
		~ScriptThread();
		
		// thread entry point
		void Run(Daemon::MessageQueue & message_queue);
		
		PyObject * PollEvent();
		
	private:		
		static bool RedirectPythonOutput(char const * filename);

		// variables
		static char const * _source_filename;
		FILE * _source_file;
		Daemon::MessageQueue * _message_queue;
	};
	
}
