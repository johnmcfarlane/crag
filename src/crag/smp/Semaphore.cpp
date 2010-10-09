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


smp::Semaphore::Semaphore(ValueType initial_value)
: sdl_semaphore(SDL_CreateSemaphore(initial_value))
{
	Assert(sdl_semaphore != nullptr);
}

smp::Semaphore::~Semaphore()
{
	Assert(sdl_semaphore != nullptr);
	SDL_DestroySemaphore(sdl_semaphore);
}

smp::Semaphore::ValueType smp::Semaphore::GetValue() const
{
	return SDL_SemValue(sdl_semaphore);
}

void smp::Semaphore::Decrement()
{
	if (SDL_SemWait(sdl_semaphore) != 0)
	{
		// Unknown error.
		Assert(false);
	}
}

bool smp::Semaphore::TryDecrement()
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

void smp::Semaphore::Increment()
{
	if (SDL_SemPost(sdl_semaphore) != 0)
	{
		// Unknown error.
		Assert(false);
	}
}
