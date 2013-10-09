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
#if ! defined(CRAG_USE_STL_THREAD)
: _thread(nullptr)
#endif
{
}

Thread::~Thread()
{
	ASSERT(! IsLaunched());
	Join();
}

bool Thread::IsLaunched() const
{
#if defined(CRAG_USE_STL_THREAD)
	return _thread.get_id() != ThreadType().get_id();
#else
	return _thread != nullptr;
#endif
}
		
// True if the calling thread is this thread.
bool Thread::IsCurrent() const
{
	if (! IsLaunched())
	{
		// This can't be the current thread if that thread isn't running.
		return false;
	}

#if defined(CRAG_USE_STL_THREAD)
	auto running_thread_id = std::this_thread::get_id();
	auto member_thread_id = _thread.get_id();
#else
	auto running_thread_id = SDL_ThreadID();
	auto member_thread_id = SDL_GetThreadID(_thread);
#endif

	return running_thread_id == member_thread_id;
}

// creates and launches a new thread;
// name must immutable
void Thread::Launch(FunctionType function, char const * name)
{
	ASSERT(! IsLaunched());
	
#if defined(CRAG_USE_STL_THREAD)
	_thread = ThreadType([this, function, name] {
		// sets the thread's name; (useful for debugging)
		smp::SetThreadName(name);

		while (! IsCurrent()) {
			Yield();
		}
		function();
	});
#else
	_launch_function = function;
	_thread = SDL_CreateThread(Callback, name, this);
#endif
}

// Waits for thread to return from FUNCTION.
void Thread::Join()
{
	// Shouldn't be called from within the thread.
	ASSERT (! IsCurrent());
	
	if (IsLaunched())
	{
#if defined(CRAG_USE_STL_THREAD)
		_thread.join();
		_thread = ThreadType();
#else
		SDL_WaitThread(_thread, nullptr);
		_thread = nullptr;
#endif
	}
}

#if ! defined(CRAG_USE_STL_THREAD)
int Thread::Callback(void * data)
{
	ASSERT(data != nullptr);
	Thread & thread = * reinterpret_cast<Thread *>(data);
	
	// Ensure that the Thread::_thread gets set before progressing.
	// This ensures that IsLaunched returns the correct result.
	while (! thread.IsLaunched())
	{
		Yield();
	}
	
	thread._launch_function();
	
	return 0;
}
#endif

