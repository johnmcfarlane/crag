//
//  Fiber.cpp
//  crag
//
//  Created by John McFarlane on 2012-03-28.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Fiber.h"

#include "core/Random.h"

#if defined(__LLP64__) || defined(__LP64__) || defined(WIN64)
// These memory model do not readily support passing of pointers.
// See [http://en.wikipedia.org/wiki/Setcontext#Example].
#define MAKECONTEXT_SMALLER_INT 1
#else
#define MAKECONTEXT_SMALLER_INT 0
#endif

#if MAKECONTEXT_SMALLER_INT
namespace
{
	// helps convert between integers and pointers
	union HelperCallbackParameters
	{
		unsigned integers[4];
		void * pointers[2];
		
		// On memory models where they are the same size, 
		// the whole excercise is ... pointless.
		static_assert(sizeof(integers) == sizeof(pointers), "This hack isn't going to work!");
	};
	
	typedef void (* MakeContextCallback) ();
	typedef void (* HelperCallback) (int, int, int, int);

	// Passed into makecontext, this function lives at the bottom of the fiber's
	// callstack. It reassembles the pointers and calls the function which was
	// passed into Fiber::Launch.	
	void helper_callback(int integer0, int integer1, int integer2, int integer3)
	{
		HelperCallbackParameters parameters;
		parameters.integers[0] = integer0;
		parameters.integers[1] = integer1;
		parameters.integers[2] = integer2;
		parameters.integers[3] = integer3;
		Fiber::Callback callback = reinterpret_cast<Fiber::Callback>(parameters.pointers[0]);
		(*callback)(parameters.pointers[1]);
	}
}
#endif

using namespace smp;

////////////////////////////////////////////////////////////////////////////////
// smp::Fiber member definitions

Fiber::Fiber(std::size_t stack_size)
: _stack_size(std::max(stack_size, std::size_t(MINSIGSTKSZ)))
, _stack(Allocate(stack_size, 1024))
{
    if (getcontext(& _context) != 0) 
	{
		DEBUG_BREAK("getcontext returned non-zero");
	}
	
	_context.uc_stack.ss_sp = _stack;
	_context.uc_stack.ss_size = _stack_size;
	_context.uc_link = nullptr;

#if defined(VERIFY)
	InitStack();
#endif
	VerifyObject(* this);
}

Fiber::~Fiber()
{
#if defined(VERIFY)
	VerifyObject(* this);
	std::size_t stack_use = EstimateStackUse();
	ASSERT(stack_use > 0);
	DEBUG_MESSAGE("fiber stack exit status: used:unused=%u:%u", 
				  static_cast<unsigned>(stack_use),
				  static_cast<unsigned>(_stack_size - stack_use));
#endif

	Free(_stack);
}

#if defined(VERIFY)
void Fiber::Verify() const
{
	ASSERT(_context.uc_stack.ss_sp != nullptr);
	ASSERT(_context.uc_stack.ss_size >= MINSIGSTKSZ);
	std::size_t stack_use = EstimateStackUse();
	ASSERT(stack_use < _stack_size - 1024);
}
#endif

void Fiber::Launch(Callback * callback, void * data)
{
	ASSERT(_context.uc_link == nullptr);

#if MAKECONTEXT_SMALLER_INT
	// makecontext only takes integers but the caller passes in a pointer
	HelperCallbackParameters parameters; 
	parameters.pointers[0] = reinterpret_cast<void *>(callback);
	parameters.pointers[1] = data;
	
	makecontext(& _context, (MakeContextCallback *)helper_callback, 4, 
			parameters.integers[0], parameters.integers[1], 
			parameters.integers[2], parameters.integers[3]);
#elif __LP64__
	static_assert(sizeof(data) == sizeof(int), "Bad parameter size. (See wikipedia entry for setcontext.)");

	makecontext(& _context, ((MakeContextCallback)callback, 1, data);
#else
// may not be too difficult to implement
#error memory model unsupported by Fiber class
#endif
}

void Fiber::Continue()
{
	ASSERT(_context.uc_link == nullptr);
	
	ucontext_t return_context;
	_context.uc_link = & return_context;
	
	swapcontext(_context.uc_link, & _context);
	
	ASSERT(_context.uc_link != nullptr);
	_context.uc_link = nullptr;
}

void Fiber::Yield()
{
	ASSERT(_context.uc_link != nullptr);
	VerifyObject(* this);
	
	swapcontext(& _context, _context.uc_link);

	ASSERT(_context.uc_link != nullptr);
	VerifyObject(* this);
}

#if ! defined(NDEBUG)
void Fiber::InitStack()
{
	// Write to each address in the stack and check these values in the d'tor.
	uintptr_t seed = reinterpret_cast<uintptr_t>(_stack);
	Random sequence(seed);
	for (uint8_t * i = reinterpret_cast<uint8_t *>(_stack), * end = i + _stack_size; i != end; ++ i)
	{
		uint8_t r = sequence.GetInt(std::numeric_limits<uint8_t>::max());
		(* i) = r;
	}
}

std::size_t Fiber::EstimateStackUse() const
{
	uintptr_t seed = reinterpret_cast<uintptr_t>(_stack);
	Random sequence(seed);
	for (uint8_t const * i = reinterpret_cast<uint8_t const *>(_stack), * end = i + _stack_size; i != end; ++ i)
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

