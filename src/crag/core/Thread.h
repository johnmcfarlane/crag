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

#if 0

#include <boost/thread.hpp>

namespace core
{
	class Thread : public boost::thread
	{
	public:
		void Join()
		{
			Super::join();
		}

					boost::thread::yield();

	private:
	};

	class Mutex : public boost::mutex
	{
	public:
	private:
	};
}

#else

namespace core
{
	class Thread 
	{
	public:
		template<typename DATA> 
		Thread(void (* cb)(DATA *), DATA * _data)
			: callback(reinterpret_cast<CallbackType>(cb))
			, data(_data)
 		{
			sdl_thread = SDL_CreateThread(OnCallback, this);
		}

		~Thread()
		{
			SDL_KillThread(sdl_thread);
		}

		void Join()
		{
			SDL_WaitThread(sdl_thread, NULL);
		}

	private:

		static int OnCallback(void * thread_ptr)
		{
			Thread * thread = reinterpret_cast<Thread *>(thread_ptr);
			(* thread->callback)(thread->data);
			return 0;
		}

		// The type this class pretends it is calling.
		typedef void (* CallbackType)(void *);

		CallbackType callback;
		void * data;
		SDL_Thread * sdl_thread;
	};

	class Mutex
	{
	public:
		Mutex()
			: sdl_mutex(SDL_CreateMutex())
		{
		}

		~Mutex()
		{
			SDL_DestroyMutex(sdl_mutex);
		}

		void Lock()
		{
			SDL_LockMutex(sdl_mutex);
		}

		void Unlock()
		{
			SDL_UnlockMutex(sdl_mutex);
		}

	private:

		SDL_mutex * sdl_mutex;
	};
}

#endif