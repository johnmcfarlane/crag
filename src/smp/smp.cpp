//
//  smp.cpp
//  crag
//
//  Created by John on 11/2/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "smp.h"

#if defined(WIN32)
#include <WinBase.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#else
#endif


void smp::Yield()
{
#if defined(WIN32)
	SleepEx(0, 0);
#else
	sched_yield();
#endif
}

void smp::Sleep(Time duration)
{
	ASSERT(duration >= 0);

#if defined(WIN32)
	Uint32 ms = static_cast<Uint32>(duration * 1000);
	SDL_Delay(ms);
#else
	// convert input to format used by nanosleep
	timespec required;
	required.tv_sec = static_cast<time_t>(duration);
	required.tv_nsec = static_cast<long>(1000000000. * (duration - required.tv_sec));
	ASSERT(NearEqual(.000000001 * required.tv_nsec + required.tv_sec, duration, 0.000001));

	// keep sleeping until the desired period is through
	timespec remaining;
	while (nanosleep(& required, & remaining))
	{
		// we require the thread to sleep for the remaining amount of time
		required = remaining;
	}
#endif
}

void smp::SetThreadPriority(int priority)
{
	// Set thread priority.
	SDL_ThreadPriority sdl_priority;
	if (priority > 0)
	{
		sdl_priority = SDL_THREAD_PRIORITY_HIGH;
	}
	else if (priority == 0)
	{
		sdl_priority = SDL_THREAD_PRIORITY_NORMAL;
	}
	else
	{
		sdl_priority = SDL_THREAD_PRIORITY_LOW;
	}
	
	SDL_SetThreadPriority(sdl_priority);
}

int smp::GetNumCpus()
{
	return SDL_GetCPUCount();
}
