//
//  FiberPosix.cpp
//  crag
//
//  Created by John McFarlane on 2012-03-28.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Fiber.h"

#if defined(CRAG_USE_FIBER_POSIX)

#include "core/Random.h"


using namespace ipc;

namespace
{
	////////////////////////////////////////////////////////////////////////////
	// file-local types

#if defined(MAKECONTEXT_SMALLER_INT)
	// helps convert between integers and pointers
	union HelperCallbackParameter
	{
		unsigned integers[2];
		void * pointer;
		
		// On memory models where they are the same size, 
		// the whole excercise is ... pointless.
		static_assert(sizeof(integers) == sizeof(pointer), "This block is only useful on systems where (int) and (void*) are different sizes.");
	};
#endif

	// function type accepted by makecontext	
	typedef void (* MakeContextCallback) ();

	////////////////////////////////////////////////////////////////////////////
	// file-local functions

	// figures out a sensible number of bytes to allocate for the fiber's stack
	std::size_t calculate_stack_allocation(std::size_t requested_stack_size)
	{
#if defined(CRAG_VERIFY_ENABLED)
		requested_stack_size = (requested_stack_size * 2) + 2048;
#endif

		// Fiber is prevented from using stacks which are less than MINSIGSTKSZ bytes
		// in size. This is in case a signal is sent to the fiber and the system
		// requires space to deal with it. We assume this doesn't happen and treat
		// MINSIGSTKSZ as a limit - not an overhead.
		return RoundToPageSize(std::max(requested_stack_size, std::size_t(MINSIGSTKSZ)));
	}
}

////////////////////////////////////////////////////////////////////////////////
// POSIX-specific ipc::Fiber member definitions

Fiber::Fiber(char const * name, std::size_t stack_size, void * data, Callback * callback)
: _name(name)
, _stack_size(stack_size)
, _is_running(true)
{
	if (getcontext(& _context) != 0) 
	{
		DEBUG_BREAK("getcontext returned non-zero");
	}

	InitContext();

#if defined(CRAG_VERIFY_ENABLED)
	InitStackUseEstimator();
#endif

	InitCallback(callback, data);

	ASSERT(IsRunning());
	ASSERT(! IsCurrent());
}

Fiber::~Fiber()
{
#if defined(CRAG_VERIFY_ENABLED)
	ASSERT(! IsRunning());
	ASSERT(! IsCurrent());
	std::size_t stack_use = EstimateStackUse();
	ASSERT(stack_use > 0);
	if (stack_use * 2 > _stack_size)
	{
		DEBUG_MESSAGE("%s stack exit status: used=%u/%u", GetName(),
					  static_cast<unsigned>(stack_use),
					  static_cast<unsigned>(_stack_size));
	}
#endif

	std::size_t actual_stack_size = calculate_stack_allocation(_stack_size);
	FreePage(_context.uc_stack.ss_sp, actual_stack_size);
}

void Fiber::InitializeThread()
{
}

bool Fiber::IsCurrent() const
{
	CRAG_VERIFY(* this);
	char * somewhereOnTheStack = reinterpret_cast<char *>(& somewhereOnTheStack);
	char * bottomOfTheStack = reinterpret_cast<char *>(_context.uc_stack.ss_sp);
	std::size_t height = somewhereOnTheStack - bottomOfTheStack;
	return height < _context.uc_stack.ss_size;
}

void Fiber::Continue()
{
	CRAG_VERIFY(* this);
	ASSERT(IsRunning());
	ASSERT(! IsCurrent());
	ASSERT(_context.uc_link == nullptr);
	
	ucontext_t return_context;
	_context.uc_link = & return_context;
	
	if (swapcontext(_context.uc_link, & _context))
	{
		DEBUG_BREAK("swapcontext returned an error");
	}
	
	ASSERT(_context.uc_link != nullptr);
	_context.uc_link = nullptr;

	ASSERT(! IsCurrent());
	CRAG_VERIFY(* this);
}

void Fiber::Yield()
{
	CRAG_VERIFY(* this);
	ASSERT(IsCurrent());
	ASSERT(_context.uc_link != nullptr);
	
	if (swapcontext(& _context, _context.uc_link))
	{
		DEBUG_BREAK("swapcontext returned an error");
	}

	ASSERT(IsRunning());
	ASSERT(IsCurrent());
	ASSERT(_context.uc_link != nullptr);
	CRAG_VERIFY(* this);
}

#if defined(CRAG_VERIFY_ENABLED)
CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Fiber, fiber)
	CRAG_VERIFY_OP(fiber._context.uc_stack.ss_sp, !=, static_cast<decltype(fiber._context.uc_stack.ss_sp)>(nullptr));
	CRAG_VERIFY_OP(fiber._context.uc_stack.ss_size, >=, static_cast<decltype(fiber._context.uc_stack.ss_size)>(MINSIGSTKSZ));
	CRAG_VERIFY_OP(fiber._context.uc_stack.ss_size, >=, fiber._stack_size);
	CRAG_VERIFY_OP(fiber._context.uc_stack.ss_size, ==, calculate_stack_allocation(fiber._stack_size));
	
	// _stack_size - and not the allocated stack size - is compared against 
	// because this value doesn't change across platforms or build configs. 
	// E.g. on OS X, allocated is always >= 32Ki so a Fiber with miniscule 
	// stack would likely never trigger as assert.
	std::size_t stack_use = fiber.EstimateStackUse();
	if (stack_use >= fiber._stack_size)
	{
		DEBUG_BREAK("%s stack overflow: used=" SIZE_T_FORMAT_SPEC "; requested:" SIZE_T_FORMAT_SPEC, fiber._name, stack_use, fiber._stack_size);
	}
	else if (stack_use >= fiber._stack_size - 1024)
	{
		DEBUG_MESSAGE("%s near stack overflow: used=" SIZE_T_FORMAT_SPEC "; requested:" SIZE_T_FORMAT_SPEC, fiber._name, stack_use, fiber._stack_size);
	}
CRAG_VERIFY_INVARIANTS_DEFINE_END

void Fiber::InitStackUseEstimator()
{
	// Write to each address in the stack and check these values in the d'tor.
	uintptr_t seed = reinterpret_cast<uintptr_t>(_context.uc_stack.ss_sp);
	Random sequence(seed & Random::SeedType(-1));
	for (uint8_t * i = reinterpret_cast<uint8_t *>(_context.uc_stack.ss_sp), * end = i +_context.uc_stack.ss_size; i != end; ++ i)
	{
		uint8_t r = sequence.GetInt(std::numeric_limits<uint8_t>::max());
		(* i) = r;
	}

	ASSERT(EstimateStackUse() == 0);
}

std::size_t Fiber::EstimateStackUse() const
{
	uintptr_t seed = reinterpret_cast<uintptr_t>(_context.uc_stack.ss_sp);
	Random sequence(seed & Random::SeedType(-1));
	for (uint8_t const * i = reinterpret_cast<uint8_t const *>(_context.uc_stack.ss_sp), * end = i + _context.uc_stack.ss_size; i != end; ++ i)
	{
		uint8_t r = sequence.GetInt(std::numeric_limits<uint8_t>::max());
		uint8_t stack_element = * i;
		if (stack_element != r)
		{
			// note: assuming that stack grows downward
			std::size_t used_count = end - i;
			return used_count;
		}
	}
	
	return 0;
}
#endif

void Fiber::InitContext()
{
	std::size_t actual_stack_size = calculate_stack_allocation(_stack_size);
	_context.uc_stack.ss_size = actual_stack_size;
	_context.uc_stack.ss_sp = AllocatePage(actual_stack_size);
	_context.uc_link = nullptr;
}

void Fiber::InitCallback(Callback * callback, void * data)
{
#if defined(MAKECONTEXT_SMALLER_INT)
	static_assert(sizeof(data) == sizeof(int) * 2, "wrong platform");

	// makecontext only takes integers but the caller passes in a pointer
	HelperCallbackParameter parameters[3];
	parameters[0].pointer = reinterpret_cast<void *>(this);
	parameters[1].pointer = reinterpret_cast<void *>(callback);
	parameters[2].pointer = data;
	
	makecontext(& _context, (MakeContextCallback)OnLaunchHelper, 6, 
			parameters[0].integers[0], parameters[0].integers[1], 
			parameters[1].integers[0], parameters[1].integers[1], 
			parameters[2].integers[0], parameters[2].integers[1]);
#else
	static_assert(sizeof(data) == sizeof(int), "wrong platform");

	int parameter0 = reinterpret_cast<int>(this);
	int parameter1 = reinterpret_cast<int>(callback);
	int parameter2 = reinterpret_cast<int>(data);
	
	makecontext(& _context, (MakeContextCallback)OnLaunch, 3, parameter0, parameter1, parameter2);
#endif
}

#if defined(MAKECONTEXT_SMALLER_INT)
void Fiber::OnLaunchHelper(unsigned i0, unsigned i1, unsigned i2, unsigned i3, unsigned i4, unsigned i5)
{
	HelperCallbackParameter parameters[3];
	parameters[0].integers[0] = i0;
	parameters[0].integers[1] = i1;
	parameters[1].integers[0] = i2;
	parameters[1].integers[1] = i3;
	parameters[2].integers[0] = i4;
	parameters[2].integers[1] = i5;
	
	Fiber & fiber = * reinterpret_cast<Fiber *>(parameters[0].pointer);
	Callback * callback = reinterpret_cast<Callback *>(parameters[1].pointer);
	void * data = reinterpret_cast<void *>(parameters[2].pointer);

	OnLaunch(fiber, callback, data);
}
#endif

void Fiber::OnLaunch(Fiber & fiber, Callback * callback, void * data)
{
#if defined(CRAG_VERIFY_ENABLED)
	CRAG_VERIFY_TRUE(fiber.IsCurrent());
	CRAG_VERIFY_OP(fiber.EstimateStackUse(), >, 0u);
#endif

	(* callback)(data);
	
	ASSERT(fiber._is_running);

	fiber._is_running = false;
	fiber.Yield();
	
	// should not have come back from the Yield call
	DEBUG_BREAK("reached end of OnLaunch for %s", fiber.GetName());
}

#endif	// ! defined(WIN32)
