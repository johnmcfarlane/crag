/*
 *  Mutex.h
 *  crag
 *
 *  Created by John on 9/26/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include <SDL_mutex.h>


namespace smp
{

	// Regular pthreads-type mutex which may sleep when locked.
	// If mutex locks for very short periods of time, consider SimpleMutex.
	// If you want to be able to try a lock, consider Semaphore.
	
	class Mutex
	{
		friend class ThreadCondition;
		
		OBJECT_NO_COPY(Mutex);

	public:
		Mutex();
		~Mutex();
		
		void Lock();
		void Unlock();
		
	private:

		SDL_mutex * sdl_mutex;
	};

}
