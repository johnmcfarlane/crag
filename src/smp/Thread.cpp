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
{
}

Thread::~Thread()
{
	Join();
}

bool Thread::IsLaunched() const
{
	return _thread.get_id() != ThreadType().get_id();
}
		
// True if the calling thread is this thread.
bool Thread::IsCurrent() const
{
	if (! IsLaunched())
	{
		// This can't be the current thread if that thread isn't running.
		return false;
	}
	
	std::thread::id running_thread_id = std::this_thread::get_id();
	std::thread::id member_thread_id = _thread.get_id();
	return running_thread_id == member_thread_id;
}

// Waits for thread to return from FUNCTION.
void Thread::Join()
{
	// Shouldn't be called from within the thread.
	ASSERT (! IsCurrent());
	
	if (IsLaunched())
	{
		_thread.join();
		_thread = ThreadType();
	}
}
