//
//  FiberAndroid.cpp
//  crag
//
//  Created by John McFarlane on 2013-04-23.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#if defined(__ANDROID__)

#include "Fiber.h"

#include "core/Random.h"

using namespace ipc;

////////////////////////////////////////////////////////////////////////////////
// MS Windows-specific ipc::Fiber member definitions

Fiber::Fiber(char const * name, std::size_t stack_size, void * data, Callback * callback)
: _name(name)
, _data(data)
, _callback(callback)
, _is_running(true)
{
	_thread = std::thread(OnLaunch, this);

	ASSERT(IsRunning());
	ASSERT(! IsCurrent());
}

Fiber::~Fiber()
{
	ASSERT(! IsRunning());
	ASSERT(! IsCurrent());
}

void Fiber::InitializeThread()
{
}

bool Fiber::IsCurrent() const
{
	VerifyObject(* this);

	auto fiber_thread_id = _thread.get_id();
	auto current_thread_id = std::this_thread::get_id();
	return fiber_thread_id == current_thread_id;
}

void Fiber::Continue()
{
	VerifyObject(* this);
	ASSERT(IsRunning());
	ASSERT(! IsCurrent());

	_condition.notify_one();

	ASSERT(! IsCurrent());
	VerifyObject(* this);
}

void Fiber::Yield()
{
	VerifyObject(* this);
	ASSERT(IsCurrent());

	// TODO: make sure that AppletInterface uses 
	// the same terminology as the wait functions
	std::unique_lock<std::mutex> lock(_condition_mutex);
	_condition.wait(lock);

	ASSERT(IsRunning());
	ASSERT(IsCurrent());
}

#if defined(VERIFY)
void Fiber::Verify() const
{
	bool joinable = _thread.joinable();
	bool is_running = IsRunning();
	VerifyEqual(joinable, is_running);
}
#endif

void Fiber::OnLaunch(Fiber * fiber)
{
	ASSERT(fiber != nullptr);
	ASSERT(fiber->IsCurrent());
	
	Callback * callback = fiber->_callback;
	void * data = fiber->_data;
	(* callback)(data);
	
	ASSERT(fiber->_is_running);

	fiber->_is_running = false;
	fiber->Yield();
	
	// should not have come back from the Yield call
	DEBUG_BREAK("reached end of OnLaunch for %s", fiber->GetName());
}

#endif	// defined(__ANDROID__)
