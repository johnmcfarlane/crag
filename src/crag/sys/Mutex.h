/*
 *  Mutex.h
 *  crag
 *
 *  Created by John on 5/28/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "SDL_mutex.h"


namespace sys
{
	class Mutex
	{
	public:
		Mutex();
		~Mutex();
		
		void Lock();
		bool TryLock();
		void Unlock();
		
	private:
		// A semaphore - not a SDL_Thread - is necessary for TryLock
		SDL_sem * sdl_semaphore;
	};
}
