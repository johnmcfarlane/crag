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
// smp::Mutex member definitions

smp::Mutex::Mutex()
: sdl_mutex(SDL_CreateMutex())
{
	Assert(sdl_mutex != nullptr);
}

smp::Mutex::~Mutex()
{
	Assert(sdl_mutex != nullptr);
	SDL_DestroyMutex(sdl_mutex);
}

void smp::Mutex::Lock()
{
	if (SDL_LockMutex(sdl_mutex) != 0)
	{
		// Unknown error.
		Assert(false);
	}
}

void smp::Mutex::Unlock()
{
	if (SDL_UnlockMutex(sdl_mutex) != 0)
	{
		// Unknown error.
		Assert(false);
	}
}
