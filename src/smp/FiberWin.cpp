//
//  Fiber.cpp
//  crag
//
//  Created by John McFarlane on 2012-03-28.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#if defined(WIN32)

#include "Fiber.h"

#include "core/Random.h"


using namespace smp;

namespace
{
	////////////////////////////////////////////////////////////////////////////
	// file-local types

	// function type accepted by makecontext	
	typedef void (* MakeContextCallback) ();

	////////////////////////////////////////////////////////////////////////////
	// file-local functions

	// figures out a sensible number of bytes to allocate for the fiber's stack
	std::size_t calculate_stack_allocation(std::size_t requested_stack_size)
	{
#if defined(VERIFY)
		requested_stack_size = (requested_stack_size * 2) + 2048;
#endif

	// Fiber is prevented from using stacks which are less than MINSIGSTKSZ bytes
	// in size. This is in case a signal is sent to the fiber and the system
	// requires space to deal with it. We assume this doesn't happen and treat
	// MINSIGSTKSZ as a limit - not an overhead.
		return requested_stack_size;
	}
}

////////////////////////////////////////////////////////////////////////////////
// MS Windows-specific smp::Fiber member definitions

Fiber::Fiber(char const * name, std::size_t stack_size, void * data, Callback * callback)
: _name(name)
, _stack_size(stack_size)
, _data(data)
, _callback(callback)
, _allocated_stack_size(calculate_stack_allocation(stack_size))
, _is_running(true)
, _fiber(CreateFiberEx(_allocated_stack_size, _allocated_stack_size, FIBER_FLAG_FLOAT_SWITCH, * OnLaunch, this))
, _calling_fiber(nullptr)
{
    if (_fiber == nullptr) 
	{
		DEBUG_BREAK("call to CreateFiberEx failed");
	}

	ASSERT(IsRunning());
	ASSERT(! IsCurrent());
}

Fiber::~Fiber()
{
	ASSERT(! IsRunning());
	ASSERT(! IsCurrent());

	DeleteFiber(_fiber);
}

void Fiber::InitializeThread()
{
	auto fiber = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
	if (fiber == nullptr)
	{
		DEBUG_BREAK("call to ConvertThreadToFiberEx failed");
	}
}

bool Fiber::IsCurrent() const
{
	VerifyObject(* this);

	return _calling_fiber != nullptr;
}

void Fiber::Continue()
{
	VerifyObject(* this);
	ASSERT(IsRunning());
	ASSERT(! IsCurrent());

	_calling_fiber = GetCurrentFiber();
	SwitchToFiber(_fiber);
	_calling_fiber = nullptr;

	ASSERT(! IsCurrent());
	VerifyObject(* this);
}

void Fiber::Yield()
{
	VerifyObject(* this);
	ASSERT(IsCurrent());

	SwitchToFiber(_calling_fiber);

	ASSERT(IsRunning());
	ASSERT(IsCurrent());
}

#if defined(VERIFY)
void Fiber::Verify() const
{
	VerifyTrue(_fiber != nullptr);
	auto current_fiber = GetCurrentFiber();
	VerifyTrue(current_fiber != _calling_fiber);
	VerifyEqual(_calling_fiber == nullptr, _fiber != current_fiber);
}
#endif

void WINAPI Fiber::OnLaunch(void * fiber_pointer)
{
	ASSERT(fiber_pointer != nullptr);
	Fiber & fiber = * static_cast<Fiber *>(fiber_pointer);
	ASSERT(fiber.IsCurrent());
	
	Callback * callback = fiber._callback;
	void * data = fiber._data;
	(* callback)(data);
	
	ASSERT(fiber._is_running);

	fiber._is_running = false;
	fiber.Yield();
	
	// should not have come back from the Yield call
	DEBUG_BREAK("reached end of OnLaunch for %s", fiber.GetName());
}

#endif	// defined(WIN32)
