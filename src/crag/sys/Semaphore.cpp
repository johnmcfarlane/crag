/*
 *  Semaphore.cpp
 *  crag
 *
 *  Created by John on 5/28/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Semaphore.h"

// core
#include "core/debug.h"


sys::Semaphore::Semaphore()
: sdl_semaphore(SDL_CreateSemaphore(1))
{
	Assert(sdl_semaphore != nullptr);
}

sys::Semaphore::~Semaphore()
{
	Assert(sdl_semaphore != nullptr);
	Assert(SDL_SemValue(sdl_semaphore) == 1);
	SDL_DestroySemaphore(sdl_semaphore);
}

void sys::Semaphore::Lock()
{
	if (SDL_SemWait(sdl_semaphore) != 0)
	{
		// Unknown error.
		Assert(false);
	}
}

bool sys::Semaphore::TryLock()
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

void sys::Semaphore::Unlock()
{
	if (SDL_SemPost(sdl_semaphore) != 0)
	{
		// Unknown error.
		Assert(false);
	}
}
