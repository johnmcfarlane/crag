//
//  Semaphore.cpp
//  crag
//
//  Created by John on 5/28/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Semaphore.h"

using namespace smp;

////////////////////////////////////////////////////////////////////////////////
// smp::Semaphore member definitions

Semaphore::Semaphore(ValueType initial_value)
: _sdl_semaphore(* SDL_CreateSemaphore(initial_value))
{
}

Semaphore::~Semaphore()
{
	SDL_DestroySemaphore(& _sdl_semaphore);
}

Semaphore::ValueType Semaphore::GetValue() const
{
	return SDL_SemValue(& _sdl_semaphore);
}

void Semaphore::Decrement()
{
	if (SDL_SemWait(& _sdl_semaphore) != 0)
	{
		CRAG_REPORT_SDL_ERROR();
	}
}

bool Semaphore::TryDecrement()
{
	int result = SDL_SemTryWait(& _sdl_semaphore);
	
	switch (result)
	{
		case 0:
			return true;
			
		case SDL_MUTEX_TIMEDOUT:
			return false;
			
		default:
			CRAG_REPORT_SDL_ERROR();
			return false;
	}
}

bool Semaphore::TryDecrement(core::Time timeout)
{
	auto milliseconds = Uint32(timeout * 1000);
	int result = SDL_SemWaitTimeout(& _sdl_semaphore, milliseconds);
	
	switch (result)
	{
		case 0:
			return true;
			
		case SDL_MUTEX_TIMEDOUT:
			return false;
			
		default:
			CRAG_REPORT_SDL_ERROR();
			return false;
	}
}

void Semaphore::Increment()
{
	if (SDL_SemPost(& _sdl_semaphore) != 0)
	{
		CRAG_REPORT_SDL_ERROR();
		::crag::core::Break();
	}
}
