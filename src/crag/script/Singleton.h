/*
 *  Singleton.h
 *  crag
 *
 *  Created by John McFarlane on 1/19/11.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Script.h"

#include "core/Singleton.h"

#include "smp/Thread.h"


namespace script
{
	
	// The scripting support is centered here.
	// The given script is run in a thread until it is done or interrupted.
	// The two ways that scripting come to an end are:
	// 1. The script finishes, in which case IsDone will return true.
	// 2. The Singleton's d'tor tells the script/thread to end.
	class Singleton : public core::Singleton<Singleton>
	{
	public:
		Singleton(char * init_source_filename);
		~Singleton();
		
		// Means that the script is done and the program should quit.
		bool IsDone() const;
		
	private:
		void Run();

		// types
		typedef smp::Thread<Singleton, & Singleton::Run> Thread;

		// attributes
		bool done;
		char * source_filename;		
		Thread thread;
	};
	
}
