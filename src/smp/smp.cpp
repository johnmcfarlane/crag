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

///////////////////////////////////////////////////////////////////////////////
// smp free function definitions

void smp::Yield()
{
#if defined(CRAG_USE_STL_THREAD)
	std::this_thread::yield();
#else
	SDL_Delay(1);
#endif
}

void smp::Sleep(core::Time seconds)
{
#if defined(CRAG_USE_STL_THREAD)
	ASSERT(seconds >= 0);

	auto microseconds = core::SecondsToDuration<std::chrono::microseconds>(seconds);
	std::this_thread::sleep_for(microseconds);
#else
	Uint32 ms = Uint32(1000. * seconds);
	SDL_Delay(ms);
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

#if defined(CRAG_OS_WINDOWS)

#include <windows.h>

// smp::SetThreadName - Windows implementation
void smp::SetThreadName(char const * thread_name)
{
	#pragma pack(push,8)
	struct THREADNAME_INFO
	{
		DWORD dwType; // Must be 0x1000.
		LPCSTR szName; // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags; // Reserved for future use, must be zero.
	};
	#pragma pack(pop)

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = thread_name;
	info.dwThreadID = 0xFFFFFFFF;	// this thread
	info.dwFlags = 0;
	
	__try
	{
		const DWORD MS_VC_EXCEPTION=0x406D1388;
		RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
}
#elif defined(CRAG_OS_X)

#include <pthread.h>

// smp::SetThreadName - OS X implementation
void smp::SetThreadName(char const * thread_name)
{
	pthread_setname_np(thread_name);
}

#elif defined(CRAG_OS_PNACL)

// smp::SetThreadName - PNaCl stub
void smp::SetThreadName(char const *)
{
}

#else

#include <sys/prctl.h>

// smp::SetThreadName - Linux implementation
void smp::SetThreadName(char const * thread_name)
{
	prctl(PR_SET_NAME, thread_name, 0, 0, 0);
}

#endif

size_t smp::GetNumCpus()
{
#if defined(CRAG_USE_STL_THREAD)
	return std::thread::hardware_concurrency();
#else
	return SDL_GetCPUCount();
#endif
}
