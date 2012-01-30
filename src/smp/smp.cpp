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
	Assert(duration >= 0);

#if defined(WIN32)
	Uint32 ms = static_cast<Uint32>(duration * 1000);
	SDL_Delay(ms);
#else
	// convert input to format used by nanosleep
	timespec required;
	required.tv_sec = static_cast<time_t>(duration);
	required.tv_nsec = static_cast<long>(1000000000. * (duration - required.tv_sec));
	Assert(NearEqual(.000000001 * required.tv_nsec + required.tv_sec, duration, 0.000001));

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

#if defined(WIN32)
//
// Usage: SetThreadName (-1, "MainThread");
//
#include <windows.h>
const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // Must be 0x1000.
   LPCSTR szName; // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void smp::SetThreadName(char const * thread_name)
{
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = thread_name;
   info.dwThreadID = -1;		// this thread
   info.dwFlags = 0;

   __try
   {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }
}
#else
void smp::SetThreadName(char const * thread_name)
{
	// OS X 10.6
	pthread_setname_np(thread_name);
	
	// Win32: http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
	// Linux: http://stackoverflow.com/questions/778085/how-to-name-a-thread-in-linux
}
#endif

int smp::GetNumCpus()
{
	return SDL_GetCPUCount();
}
