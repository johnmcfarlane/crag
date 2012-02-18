//
//  Fiber.h
//  crag
//
//  Created by John McFarlane on 2012-02-06.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "FiberInterface.h"
#include "StackCushion.h"
#include "StackFrameBuffer.h"
#include "TimeCondition.h"

#include <setjmp.h>


namespace script
{
	// Crude implementation of a Fiber intended to help serve as 
	// an alternative to the threadlets in Stackless Python.
	class Fiber : public FiberInterface
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Fiber();
		~Fiber();
		
		bool IsComplete() const;
		bool TestCondition(ScriptThread & scrip_thread) const;

		void Continue();
		void Start(ScriptThread & script_thread);
	private:
		void InternalStart();

		// FiberInterface 
		bool GetQuitFlag() const override;
		void SetQuitFlag() override;

		void Yield() override;
		void Sleep(Time duration) override;
		void Wait(Condition & condition) override;
		
		void LaunchFiber(Fiber & fiber);
		
		virtual void Run() = 0;
		
		// true iff it's safe to access this object when fiber is paused
		template <typename OBJECT>
		bool IsPersistent(OBJECT const & object);
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		ScriptThread * _script_thread;
		DEFINE_INTRUSIVE_LIST(Fiber, List);
		jmp_buf _source_state;
		jmp_buf _dest_state;
		StackFrameBuffer _stack_frame_buffer;
		TimeCondition _time_condition;	// used often enough to warrant its own instance
		Condition * _condition;
		bool _complete;
	};
	
	// class which binds a fiber and its functor
	template <typename FUNCTOR>
	class FiberEntry : public Fiber
	{
	public:
		FiberEntry(FUNCTOR const & functor)
		: _functor(functor)
		{
		}
		
	private:
		void Run() override
		{
			_functor(* this);
		}
		
		FUNCTOR _functor;
	};
	
	// definition of FiberInterface function requiring defition of FiberEntry
	template <typename FUNCTOR>
	void FiberInterface::Launch(FUNCTOR const & functor)
	{
		LaunchFiber(ref(new FiberEntry<FUNCTOR>(functor)));
	}
}
