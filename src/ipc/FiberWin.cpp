//
//  FiberWin.cpp
//  crag
//
//  Created by John McFarlane on 2012-03-28.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Fiber.h"

#if defined(CRAG_USE_FIBER_WIN)

#include "core/Random.h"


using namespace ipc;

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
#if defined(CRAG_VERIFY_ENABLED)
		// add some headroom in non-release builds
		requested_stack_size = (requested_stack_size * 2) + 2048;
#endif

		return requested_stack_size;
	}
}

////////////////////////////////////////////////////////////////////////////////
// MS Windows-specific ipc::Fiber member definitions

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
	CRAG_VERIFY(* this);

	return _calling_fiber != nullptr;
}

void Fiber::Continue()
{
	CRAG_VERIFY(* this);
	ASSERT(IsRunning());
	ASSERT(! IsCurrent());

	_calling_fiber = GetCurrentFiber();
	SwitchToFiber(_fiber);
	_calling_fiber = nullptr;

	ASSERT(! IsCurrent());
	CRAG_VERIFY(* this);
}

void Fiber::Yield()
{
	CRAG_VERIFY(* this);
	ASSERT(IsCurrent());

	SwitchToFiber(_calling_fiber);

	ASSERT(IsRunning());
	ASSERT(IsCurrent());
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Fiber, self)
	CRAG_VERIFY_TRUE(self._fiber != nullptr);
	auto current_fiber = GetCurrentFiber();
	CRAG_VERIFY_TRUE(current_fiber != self._calling_fiber);
	CRAG_VERIFY_EQUAL(self._calling_fiber == nullptr, self._fiber != current_fiber);
CRAG_VERIFY_INVARIANTS_DEFINE_END

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

#endif	// defined(CRAG_OS_WINDOWS)
