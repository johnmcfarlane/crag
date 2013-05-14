//
//  FiberAndroid.cpp
//  crag
//
//  Created by John McFarlane on 2013-04-23.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Fiber.h"

#if defined(__ANDROID__)

#include "smp/smp.h"

using namespace ipc;

////////////////////////////////////////////////////////////////////////////////
// MS Windows-specific ipc::Fiber member definitions

Fiber::Fiber(char const * name, std::size_t /*stack_size*/, void * data, Callback * callback)
: _name(name)
, _is_running(true)
, _is_yielded(true)
{
	_thread.Launch([=] () { OnLaunch(this, data, callback); }, name);

	ASSERT(IsRunning());
	ASSERT(! IsCurrent());
}

Fiber::~Fiber()
{
	_thread.Join();
	
	ASSERT(! IsRunning());
	ASSERT(! IsCurrent());
}

void Fiber::InitializeThread()
{
}

bool Fiber::IsCurrent() const
{
	VerifyObject(* this);

	return _thread.IsCurrent();
}

void Fiber::Continue()
{
	VerifyObject(* this);
	ASSERT(IsRunning());
	ASSERT(! IsCurrent());

	_is_yielded = false;
	_condition.notify_one();
	
	std::unique_lock<std::mutex> lock(_condition_mutex);
	_condition.wait(lock, [&] () { return _is_yielded; });

	ASSERT(! IsCurrent());
	VerifyObject(* this);
}

void Fiber::Yield()
{
	VerifyObject(* this);
	ASSERT(IsCurrent());
	
	_is_yielded = true;
	_condition.notify_one();

	// TODO: make sure that AppletInterface uses 
	// the same terminology as the wait functions
	std::unique_lock<std::mutex> lock(_condition_mutex);
	_condition.wait(lock, [&] () { return ! _is_yielded; });

	ASSERT(IsRunning());
	ASSERT(IsCurrent());
}

#if defined(VERIFY)
void Fiber::Verify() const
{
	VerifyTrue(_name != nullptr);
	if (_is_yielded)
	{
		VerifyTrue(! _thread.IsCurrent());
	}
	else
	{
		if (! _is_running)
		{
			// means we're yielding for the last time
			VerifyTrue(_thread.IsCurrent());
		}
	}
}
#endif

void Fiber::OnLaunch(Fiber * fiber, void * data, Callback * callback)
{
	ASSERT(fiber != nullptr);
	ASSERT(data != nullptr);
	ASSERT(callback != nullptr);
	
	fiber->_is_yielded = false;
	ASSERT(fiber->IsCurrent());
	
	(* callback)(data);
	
	ASSERT(fiber->_is_running);

	fiber->_is_running = false;
	fiber->_is_yielded = true;
	fiber->_condition.notify_one();
}

#endif	// defined(__ANDROID__)
