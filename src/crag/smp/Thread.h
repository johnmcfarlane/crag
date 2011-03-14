/*
 *  Sphere.h
 *  Crag
 *
 *  Created by John on 10/31/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "smp.h"


namespace smp
{

	// Use Thread to launch and manage a thread. 
	// Specifically, Thread can be used to call a member function from within a newly launched thread.
	// Template parameter, CLASS, is the class and FUNCTION is the member function to launch into.
	// The function parameter, object, is the object for which the member function is called.
	// (Note that two other useful thread-related functions are Sleep and GetNumCpus.)

	// WARNING: Immediately after launching the new thread it is wise to Sleep that thread. TODO: Is it?
	// WARNING: Otherwise, various of the Thread member functions will return incorrect results.
	// WARNING: This is because the internal state of the class is updated on return from SDL_CreateThread.

	template <typename CLASS> 
	class Thread 
	{
		OBJECT_NO_COPY(Thread);
		
	public:
		
		Thread()
		: sdl_thread(nullptr)
		{
		}
		
		~Thread()
		{
			Join();
		}
		
		// Note: The thread may have quit but this will still return true until Kill or Join is called.
		bool IsLaunched() const
		{
			return sdl_thread != nullptr;
		}
		
		// True if the calling thread is this thread.
		bool IsCurrent() const
		{
			if (! IsLaunched())
			{
				// This can't be the current thread if that thread isn't running.
				return false;
			}
			
			SDL_threadID running_thread_id = SDL_ThreadID();
			SDL_threadID member_thread_id = SDL_GetThreadID(sdl_thread);
			return running_thread_id == member_thread_id;
		}

		// Creates and launches a new thread which calls object.FUNCTION().
		// If the thread is already running, it is forcefully stopped first.
		template <void (CLASS::*FUNCTION)()>
		void Launch(CLASS & object)
		{
			// If launched already, wait to stop being launched.
			Join();
			
			// Call the given FUNCTION, passing given object, in a new thread. 
			sdl_thread = SDL_CreateThread(Callback<FUNCTION>, reinterpret_cast<void *>(& object));
		}
		
		// Terminates the thread. Risks losing data the thread is working on.
		// If possible, try and use Join instead. 
		void Kill()
		{
			// Shouldn't be called from within the thread.
			Assert (! IsCurrent());
			
			if (sdl_thread != nullptr)
			{
				SDL_KillThread(sdl_thread);
				sdl_thread = nullptr;
			}
		}
		
		// Waits for thread to return from FUNCTION.
		void Join()
		{
			// Shouldn't be called from within the thread.
			Assert (! IsCurrent());
			
			if (sdl_thread != nullptr)
			{
				SDL_WaitThread(sdl_thread, NULL);
				sdl_thread = nullptr;
			}
		}

	private:
		
		template <void (CLASS::*FUNCTION)()>
		static int Callback(void * data)
		{
			Assert(data != nullptr);
			Sleep(0);
			CLASS & object = * reinterpret_cast<CLASS *> (data);
			(object.*FUNCTION)();
			return 0;
		}
		
		SDL_Thread * sdl_thread;
	};
	
}
