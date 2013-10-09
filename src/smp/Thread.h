//
//  Thread.h
//  crag
//
//  Created by John McFarlane on 2011/07/25.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "smp.h"

// use std::thread for smp::Thread implementation 
// instead of SDL threadding support; both use pthreads on most systems
// but SDL is a little more mature and claims to work better on Android
//#define CRAG_USE_STL_THREAD

#if defined(CRAG_USE_STL_THREAD)
#include <thread>
#endif

namespace smp
{
	// Use Thread to launch and manage a thread. 
	// Specifically, Thread can be used to call a member function from within a newly launched thread.
	// Template parameter, CLASS, is the class and FUNCTION is the member function to launch into.
	// The function parameter, object, is the object for which the member function is called.
	// (Note that two other useful thread-related functions are Sleep and GetNumCpus.)

	class Thread 
	{
		OBJECT_NO_COPY(Thread);
		
		// types
#if defined(CRAG_USE_STL_THREAD)
		typedef std::thread ThreadType;
#else
		typedef SDL_Thread * ThreadType;
#endif
		typedef std::function<void ()> FunctionType;

	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		Thread();
		~Thread();
		
		bool IsLaunched() const;
		
		// True if the calling thread is this thread.
		bool IsCurrent() const;

		// creates and launches a new thread;
		// name must immutable
		void Launch(FunctionType function, char const * name);
		
		// Waits for thread to return from FUNCTION.
		void Join();

	private:
#if ! defined(CRAG_USE_STL_THREAD)
		static int Callback(void * data);
#endif

		////////////////////////////////////////////////////////////////////////////////
		// variables

		ThreadType _thread;

#if ! defined(CRAG_USE_STL_THREAD)
		FunctionType _launch_function;
#endif
	};
}
