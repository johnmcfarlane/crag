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

using namespace smp;


////////////////////////////////////////////////////////////////////////////////
// smp::Fiber member definitions

Fiber::Fiber(std::size_t stack_size)
: _stack_size(std::max(stack_size, std::size_t(MINSIGSTKSZ)))
, _stack(Allocate(stack_size, 1024))
, _functor_wrapper(nullptr)
{
    if (getcontext(& _context) != 0) 
	{
		DEBUG_BREAK("getcontext returned non-zero");
	}
	
	_context.uc_stack.ss_sp = _stack;
	_context.uc_stack.ss_size = _stack_size;
	_context.uc_link = nullptr;

	InitStack();
	VerifyObject(* this);
}

Fiber::~Fiber()
{
#if ! defined (NDEBUG)
	VerifyObject(* this);
	std::size_t stack_use = EstimateStackUse();
	ASSERT(stack_use > 0);
	DEBUG_MESSAGE("fiber stack exit status: used:unused=%u:%u", 
				  static_cast<unsigned>(stack_use),
				  static_cast<unsigned>(_stack_size - stack_use));
#endif

	if (IsRunning())
	{
		DEBUG_BREAK("Forcing shutdown of Fiber");
		Kill();
	}
	
	Free(_stack);
}

#if defined(VERIFY)
void Fiber::Verify() const
{
	ASSERT(_context.uc_stack.ss_sp != nullptr);
	ASSERT(_context.uc_stack.ss_size >= MINSIGSTKSZ);
	std::size_t stack_use = EstimateStackUse();
	ASSERT(stack_use < _stack_size - 1024);
	
	if (! IsRunning())
	{
		ASSERT(_context.uc_link == nullptr);
	}
}
#endif

bool Fiber::IsRunning() const
{
	return _functor_wrapper != nullptr;
}

void Fiber::Launch(FunctorWrapperBase & functor_wrapper)
{
    ASSERT(! IsRunning());
	ASSERT(_context.uc_link == nullptr);

	_functor_wrapper = & functor_wrapper;
	makecontext(& _context, (void (*)()) OnLaunch, 1, this);
}

void Fiber::Continue()
{
	ASSERT(_context.uc_link == nullptr);
	if (! IsRunning())
	{
		return;
	}
	
	ucontext_t return_context;
	_context.uc_link = & return_context;
	
	swapcontext(_context.uc_link, & _context);
	
	ASSERT(_context.uc_link != nullptr);
	_context.uc_link = nullptr;
}

// entry point for fiber
void Fiber::OnLaunch(Fiber * fiber)
{
	ASSERT(fiber != nullptr);
	ASSERT(fiber->_context.uc_link != nullptr);
	ASSERT(fiber->_functor_wrapper != nullptr);
	
	fiber->_functor_wrapper->OnLaunch(* fiber);
	fiber->Kill();

	ASSERT(fiber->_context.uc_link != nullptr);
	ASSERT(fiber->_functor_wrapper == nullptr);
}

void Fiber::Yield()
{
	ASSERT(_context.uc_link != nullptr);
	VerifyObject(* this);
	
	swapcontext(& _context, _context.uc_link);

	ASSERT(_context.uc_link != nullptr);
	VerifyObject(* this);
}

void Fiber::Kill()
{
	if (_functor_wrapper == nullptr) 
	{
		DEBUG_BREAK("_functor_wrapper=%p", _functor_wrapper);
		return;
	}

	delete _functor_wrapper;
	_functor_wrapper = nullptr;
}

void Fiber::InitStack()
{
#if ! defined(NDEBUG)
	// Write to each address in the stack and check these values in the d'tor.
	Random sequence(reinterpret_cast<uint32_t>(_stack));
	for (uint8_t * i = reinterpret_cast<uint8_t *>(_stack), * end = i + _stack_size; i != end; ++ i)
	{
		uint8_t r = sequence.GetInt(std::numeric_limits<uint8_t>::max());
		(* i) = r;
	}
#endif
}

std::size_t Fiber::EstimateStackUse() const
{
	Random sequence(reinterpret_cast<uint32_t>(_stack));
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
