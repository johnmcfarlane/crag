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

using namespace smp;


////////////////////////////////////////////////////////////////////////////////
// smp::Fiber member definitions

Fiber::Fiber(std::size_t stack_size)
: _functor_wrapper(nullptr)
{
    if (getcontext(& _context) != 0) 
	{
		DEBUG_BREAK("getcontext returned non-zero");
	}
	
	stack_size += MINSIGSTKSZ;
	_stack = Allocate(stack_size, 1024);
	
	_context.uc_stack.ss_sp = _stack;
	_context.uc_stack.ss_size = stack_size;
	_context.uc_link = nullptr;
}

Fiber::~Fiber()
{
	if (IsRunning())
	{
		DEBUG_BREAK("Forcing shutdown of Fiber");
		Kill();
	}
	
	Free(_stack);

	ASSERT(_context.uc_stack.ss_sp != nullptr);
	ASSERT(_context.uc_stack.ss_size >= MINSIGSTKSZ);
}

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
	
	swapcontext(& _context, _context.uc_link);

	ASSERT(_context.uc_link != nullptr);
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
