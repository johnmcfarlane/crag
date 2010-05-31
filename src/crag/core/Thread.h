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
			sdl_thread = SDL_CreateThread(Launch, this);
		}

		~Thread();
		
		bool IsCurrent() const;

		void Join();

	private:
		static int Launch(void * thread_ptr);

		// The type this class pretends it is calling.
		typedef void (* CallbackType)(void *);

		CallbackType callback;
		void * data;
		SDL_Thread * sdl_thread;
	};
}
