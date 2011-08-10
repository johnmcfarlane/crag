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
		
	public:
		// types
		typedef int (* Function)(void * data);
		
		// functions
		Thread();
		~Thread();
		
		// Note: The thread may have quit but this will still return true until Kill or Join is called.
		bool IsLaunched() const;
		
		// True if the calling thread is this thread.
		bool IsCurrent() const;

		// Creates and launches a new thread.
		void Launch(Function callback, void * data);
		
		// Object-oriented thread launch.
		template <typename CLASS, Thread CLASS::*THREAD, void (CLASS::*FUNCTION)()>
		static void Launch(CLASS & object)
		{
			Thread & thread = object.*THREAD;
			
			// Never called from within the thread.
			Assert(! thread.IsCurrent());
			
			thread.Launch(& Callback<CLASS, THREAD, FUNCTION>, & object);
		}
		
		// Terminates the thread. Risks losing data the thread is working on.
		// If possible, try and use Join instead. 
		void Kill();
		
		// Waits for thread to return from FUNCTION.
		void Join();

	private:
		template <typename CLASS, Thread CLASS::*THREAD, void (CLASS::*FUNCTION)()>
		static int Callback(void * data)
		{
			Assert(data != nullptr);
			CLASS & object = * reinterpret_cast<CLASS *>(data);
			
			// Ensure that the Thread::sdl_thread gets set before progressing.
			// This ensures that IsLaunched returns the correct result.
			Thread const & object_thread = object.*THREAD;
			while (! object_thread.IsLaunched())
			{
				Sleep(0);
			}
			
			// Call the class' member function.
			(object.*FUNCTION)();
			
			return 0;
		}

		SDL_Thread * sdl_thread;
	};
}
