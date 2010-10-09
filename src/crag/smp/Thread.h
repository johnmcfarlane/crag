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

#include <SDL_thread.h>


namespace smp
{

	// Use Thread to launch and manage a thread. 
	// Specifically, Thread can be used to call a member function from within a newly launched thread.
	// Template parameter, CLASS, is the class and FUNCTION is the member function to launch into.
	// The function parameter, object, is the object for which the member function is called.
	// (Note that two other useful thread-related functions are Sleep and GetNumCpus.)

	template <typename CLASS, void (CLASS::*FUNCTION)()> 
	class Thread 
	{
		OBJECT_NO_COPY(Thread);		
	public:
	
		// Creates and launches a new thread which calls object.FUNCTION().
		Thread (CLASS & object)
		: sdl_thread(nullptr)
		{
			Launch(object);
		}
		
		Thread()
		: sdl_thread(nullptr)
		{
		}
		
		~Thread()
		{
			Kill();
		}
		
		// True if the calling thread is this thread.
		// TODO: This doesn't seem to work 100% of the time.
		bool IsCurrent() const
		{
			Uint32 running_thread_id = SDL_ThreadID();
			Uint32 member_thread_id = SDL_GetThreadID(sdl_thread);
			return running_thread_id == member_thread_id;
		}

		// Creates and launches a new thread which calls object.FUNCTION().
		// If the thread is already running, it is forcefully stopped first.
		void Launch(CLASS & object)
		{
			Kill();
			sdl_thread = SDL_CreateThread(Callback, reinterpret_cast<void *>(& object));
		}
		
		// Terminates the thread. Risks losing data the thread is working on.
		// If possible, try and use Join instead. 
		void Kill()
		{
			if (sdl_thread != nullptr)
			{
				SDL_KillThread(sdl_thread);
				sdl_thread = nullptr;
			}
		}
		
		// Waits for thread to return from FUNCTION.
		void Join()
		{
			if (sdl_thread != nullptr)
			{
				SDL_WaitThread(sdl_thread, NULL);
				sdl_thread = nullptr;
			}
		}

	private:
		
		static int Callback(void * data)
		{
			Assert(data != nullptr);
			CLASS & object = * reinterpret_cast<CLASS *> (data);
			(object.*FUNCTION)();
			return 0;
		}
		
		SDL_Thread * sdl_thread;
	};
	
}
