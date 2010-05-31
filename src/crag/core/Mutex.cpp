/*
 *  Mutex.cpp
 *  crag
 *
 *  Created by John on 5/28/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "Mutex.h"

// core
#include "core/debug.h"


core::Mutex::Mutex()
: sdl_semaphore(SDL_CreateSemaphore(1))
{
	Assert(sdl_semaphore != nullptr);
}

core::Mutex::~Mutex()
{
	Assert(sdl_semaphore != nullptr);
	Assert(SDL_SemValue(sdl_semaphore) == 1);
	SDL_DestroySemaphore(sdl_semaphore);
}

void core::Mutex::Lock()
{
	if (SDL_SemWait(sdl_semaphore) != 0)
	{
		// Unknown error.
		Assert(false);
	}
}

bool core::Mutex::TryLock()
{
	int result = SDL_SemTryWait(sdl_semaphore);
	
	switch (result)
	{
		case 0:
			return true;
			
		case SDL_MUTEX_TIMEDOUT:
			return false;
			
		default:
			// Unknown error.
			Assert(false);
			return false;
	}
}

void core::Mutex::Unlock()
{
	if (SDL_SemPost(sdl_semaphore) != 0)
	{
		// Unknown error.
		Assert(false);
	}
}
