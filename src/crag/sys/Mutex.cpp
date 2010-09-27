/*
 *  Mutex.cpp
 *  crag
 *
 *  Created by John on 9/26/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Mutex.h"

#include "core/debug.h"


////////////////////////////////////////////////////////////////////////////////
// sys::Mutex member definitions

sys::Mutex::Mutex()
: sdl_mutex(SDL_CreateMutex())
{
	Assert(sdl_mutex != nullptr);
}

sys::Mutex::~Mutex()
{
	Assert(sdl_mutex != nullptr);
	SDL_DestroyMutex(sdl_mutex);
}

void sys::Mutex::Lock()
{
	if (SDL_LockMutex(sdl_mutex) != 0)
	{
		// Unknown error.
		Assert(false);
	}
}

void sys::Mutex::Unlock()
{
	if (SDL_UnlockMutex(sdl_mutex) != 0)
	{
		// Unknown error.
		Assert(false);
	}
}
