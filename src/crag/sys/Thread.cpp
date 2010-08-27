/*
 *  Thread.cpp
 *  crag
 *
 *  Created by John on 5/28/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Thread.h"


sys::Thread::~Thread()
{
	SDL_KillThread(sdl_thread);
}

bool sys::Thread::IsCurrent() const
{
	Uint32 current_thread_id = SDL_ThreadID();
	Uint32 thread_id = SDL_GetThreadID(sdl_thread);
	return current_thread_id == thread_id;
}

void sys::Thread::Join()
{
	SDL_WaitThread(sdl_thread, NULL);
}

int sys::Thread::Launch(void * thread_ptr)
{
	Thread * thread = reinterpret_cast<Thread *>(thread_ptr);
	(* thread->callback)(thread->data);
	return 0;
}
