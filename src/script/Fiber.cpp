//
//  Fiber.cpp
//  crag
//
//  Created by John McFarlane on 2012-02-06.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Fiber.h"

#include "Script.h"
#include "ScriptThread.h"


using namespace script;


namespace
{
	// condition which never fails;
	// used by Yield to return ASAP;
	// TODO: Should probably not be needed.
	class NullCondition : public Condition
	{
		bool operator() (ScriptThread & script_thread)
		{
			return true;
		}
	};
	
	NullCondition null_condition;
}


////////////////////////////////////////////////////////////////////////////////
// script::Fiber member definitions

Fiber::Fiber(Script & script)
: _script_thread(nullptr)
, _condition(nullptr)
, _script(script)
, _complete(false)
{
}

Fiber::~Fiber()
{
	ASSERT(_complete == true);
	delete & _script;
}

bool Fiber::IsComplete() const
{
	return _complete;
}

Script & Fiber::GetScript()
{
	return _script;
}

Condition * Fiber::GetCondition()
{
	return _condition;
}

void Fiber::Continue()
{
	if (! setjmp(_source_state))
	{
		// Restore the copy of the vulnerable part of the stack.
		_stack_frame_buffer.Restore();
		
		// Jump into fiber.
		longjmp(_dest_state, 1);
		
		// Should never return...
		ASSERT(false);
	}
	else
	{
		// ...but arrives here when fiber yields.
	}
}

void Fiber::Start(ScriptThread & script_thread)
{
	ASSERT(_script_thread == nullptr);
	_script_thread = & script_thread;
	
	if (! setjmp(_source_state))
	{
		InternalStart();
		
		// Should never return.
		ASSERT(false);
	}
	else
	{
		// Passes through here when the fiber yields.
	}
}

void Fiber::InternalStart()
{
	StackCushion cushion;
	
	// TODO: Marker may need to be lower than stack frame - rather than within it.
	char stack_marker;
	_stack_frame_buffer.SetBottom(& stack_marker);
	
	Run();
	
	// Flag that this fiber is done.
	_complete = true;
	
	if (! cushion.IsSafe())
	{
		ERROR_MESSAGE("Warning: Evidence of fiber stack corruption detected.");
		ASSERT(false);
	}
	
	// Yield for the last time.
	Yield();
	
	// Should not come back from that yield.
	ASSERT(false);
}

bool Fiber::GetQuitFlag() const
{
	return _script_thread->GetQuitFlag();
}

void Fiber::SetQuitFlag()
{
	_script_thread->SetQuitFlag();
}

void Fiber::Yield()
{
	Wait(null_condition);
}

void Fiber::Sleep(Time duration)
{
	_time_condition.SetWakePosition(_script_thread->GetTime() + duration);
	Wait(_time_condition);
}

void Fiber::Wait(Condition & condition)
{
	ASSERT(_condition == nullptr);
	_condition = & condition;

	// TODO: Marker may need to be higher than stack frame - rather than within it.
	_stack_frame_buffer.SetTop(StackFrameBuffer::CalculateTop());
	
	if (! setjmp(_dest_state))
	{
		// Make a copy of the vulnerable part of the stack.
		_stack_frame_buffer.Copy();
		
		// Jump back to caller.
		longjmp(_source_state, 1);
		
		// Should never return...
		ASSERT(false);
	}
	else
	{
		// ... but passes through here when yield is complete.
	}
	
	// For safety's sake un-set the top pointer.
	_stack_frame_buffer.SetTop(StackFrameBuffer::null);
	
	ASSERT(_condition == & condition);
	_condition = nullptr;
}

void Fiber::Launch(Script & script)
{
	_script_thread->OnAddObject(script);
	Yield();
}

void Fiber::Run()
{
	_script(* this);
}

template <typename OBJECT>
bool Fiber::IsPersistent(OBJECT const & object)
{
	return _stack_frame_buffer.IsPersistent(object);
}
