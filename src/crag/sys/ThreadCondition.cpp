/*
 *  ThreadCondition.cpp
 *  crag
 *
 *  Created by John on 9/24/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Mutex.h"
#include "ThreadCondition.h"


////////////////////////////////////////////////////////////////////////////////
// sys::ThreadCondition member definitions

sys::ThreadCondition::ThreadCondition ()
: sdl_condition(SDL_CreateCond())
{
	Assert(sdl_condition != nullptr);
}

sys::ThreadCondition::~ThreadCondition()
{
	Assert(sdl_condition != nullptr);
	SDL_DestroyCond(sdl_condition);
}

// Gracefully waits for the thread to end.
void sys::ThreadCondition::Wait(sys::Mutex & mutex)
{
	if (SDL_CondWait(sdl_condition, mutex.sdl_mutex) != 0)
	{
		Assert(false);
	}
}

// Gracefully waits for the thread to end.
void sys::ThreadCondition::Wait(sys::Mutex & mutex, TimeType timeout)
{
	Uint32 ms = static_cast<Uint32> (timeout * 1000);
	if (SDL_CondWaitTimeout(sdl_condition, mutex.sdl_mutex, ms) != 0)
	{
		Assert(false);
	}
}

void sys::ThreadCondition::Restart()
{
	if (SDL_CondSignal(sdl_condition) != 0)
	{
		Assert(false);
	}
}

void sys::ThreadCondition::RestartAll()
{
	if (SDL_CondBroadcast(sdl_condition) != 0)
	{
		Assert(false);
	}
}
