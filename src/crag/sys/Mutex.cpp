/*
 *  Mutex.cpp
 *  crag
 *
 *  Created by John on 5/28/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Mutex.h"

// core
#include "core/debug.h"


sys::Mutex::Mutex()
: sdl_semaphore(SDL_CreateSemaphore(1))
{
	Assert(sdl_semaphore != nullptr);
}

sys::Mutex::~Mutex()
{
	Assert(sdl_semaphore != nullptr);
	Assert(SDL_SemValue(sdl_semaphore) == 1);
	SDL_DestroySemaphore(sdl_semaphore);
}

void sys::Mutex::Lock()
{
	if (SDL_SemWait(sdl_semaphore) != 0)
	{
		// Unknown error.
		Assert(false);
	}
}

bool sys::Mutex::TryLock()
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

void sys::Mutex::Unlock()
{
	if (SDL_SemPost(sdl_semaphore) != 0)
	{
		// Unknown error.
		Assert(false);
	}
}