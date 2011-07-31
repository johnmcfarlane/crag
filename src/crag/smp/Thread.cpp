//
//  Thread.cpp
//  crag
//
//  Created by John McFarlane on 2011/07/25.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Thread.h"


using namespace smp;

Thread::Thread()
: sdl_thread(nullptr)
{
}

Thread::~Thread()
{
	Join();
}

// Note: The thread may have quit but this will still return true until Kill or Join is called.
bool Thread::IsLaunched() const
{
	return sdl_thread != nullptr;
}
		
// True if the calling thread is this thread.
bool Thread::IsCurrent() const
{
	if (! IsLaunched())
	{
		// This can't be the current thread if that thread isn't running.
		return false;
	}
	
	SDL_threadID running_thread_id = SDL_ThreadID();
	SDL_threadID member_thread_id = SDL_GetThreadID(sdl_thread);
	return running_thread_id == member_thread_id;
}

void Thread::Launch(Function callback, void * data)
{
	// If launched already, wait to stop being launched.
	Join();
			
	// Call the given FUNCTION, passing given object, in a new thread. 
	sdl_thread = SDL_CreateThread(callback, data);
}

// Terminates the thread. Risks losing data the thread is working on.
// If possible, try and use Join instead. 
void Thread::Kill()
{
	// Shouldn't be called from within the thread.
	Assert (! IsCurrent());
			
	if (sdl_thread != nullptr)
	{
		SDL_KillThread(sdl_thread);
		sdl_thread = nullptr;
	}
}
		
// Waits for thread to return from FUNCTION.
void Thread::Join()
{
	// Shouldn't be called from within the thread.
	Assert (! IsCurrent());
	
	if (sdl_thread != nullptr)
	{
		SDL_WaitThread(sdl_thread, NULL);
		sdl_thread = nullptr;
	}
}
