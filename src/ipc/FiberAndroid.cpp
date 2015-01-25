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

#if defined(CRAG_USE_FIBER_THREADED)

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
	CRAG_VERIFY(* this);

	return _thread.IsCurrent();
}

void Fiber::Continue()
{
	CRAG_VERIFY(* this);
	ASSERT(IsRunning());
	ASSERT(! IsCurrent());

	_is_yielded = false;
	_condition.notify_one();
	
	std::unique_lock<std::mutex> lock(_condition_mutex);
	_condition.wait(lock, [&] () { return _is_yielded; });

	ASSERT(! IsCurrent());
	CRAG_VERIFY(* this);
}

void Fiber::Yield()
{
	CRAG_VERIFY(* this);
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

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Fiber, self)
	CRAG_VERIFY_TRUE(self._name != nullptr);
	if (self._is_yielded)
	{
		CRAG_VERIFY_TRUE(! self._thread.IsCurrent());
	}
	else
	{
		if (! self._is_running)
		{
			// means we're yielding for the last time
			CRAG_VERIFY_TRUE(self._thread.IsCurrent());
		}
	}
CRAG_VERIFY_INVARIANTS_DEFINE_END

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

#endif	// defined(CRAG_USE_FIBER_THREADED)
