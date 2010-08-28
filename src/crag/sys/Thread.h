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


namespace sys
{

	// TODO: Describe how to use this class. It's a bit unusual. 
	template <typename CLASS, void (CLASS::*FUNCTION)()> 
	class Thread 
	{
	public:
		
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
			Uint32 current_thread_id = SDL_ThreadID();
			Uint32 thread_id = SDL_GetThreadID(sdl_thread);
			return current_thread_id == thread_id;
		}

		// Start the thread, calling the callback.
		void Launch(CLASS & object)
		{
			Kill();
			sdl_thread = SDL_CreateThread(Callback, reinterpret_cast<void *>(& object));
		}
		
		// Terminates the thread. Risks losing data the thread is working on.
		void Kill()
		{
			if (sdl_thread != nullptr)
			{
				SDL_KillThread(sdl_thread);
				sdl_thread = nullptr;
			}
		}
		
		// Gracefully waits for the thread to end.
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
