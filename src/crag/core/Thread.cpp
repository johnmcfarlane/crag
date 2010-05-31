/*
 *  Thread.cpp
 *  crag
 *
 *  Created by John on 5/28/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "Thread.h"


core::Thread::~Thread()
{
	SDL_KillThread(sdl_thread);
}

bool core::Thread::IsCurrent() const
{
	Uint32 current_thread_id = SDL_ThreadID();
	Uint32 thread_id = SDL_GetThreadID(sdl_thread);
//	std::cout << "current_thread_id=" << current_thread_id << '\n';
//	std::cout << "thread_id=" << thread_id << '\n';
	return current_thread_id == thread_id;
}

void core::Thread::Join()
{
	SDL_WaitThread(sdl_thread, NULL);
}

int core::Thread::Launch(void * thread_ptr)
{
	Thread * thread = reinterpret_cast<Thread *>(thread_ptr);
	(* thread->callback)(thread->data);
	return 0;
}
