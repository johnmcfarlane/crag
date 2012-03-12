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
		
		Fiber(Script & script);	// takes ownership of this script
		~Fiber();
		
		bool IsComplete() const;
		Script & GetScript();
		Condition * GetCondition();
		
		void Continue();
		void Start(ScriptThread & script_thread);
	private:
		void InternalStart();
		
		// FiberInterface 
		virtual bool GetQuitFlag() const override;
		virtual void SetQuitFlag() override;
		
		virtual void Yield() override;
		virtual void Sleep(Time duration) override;
		virtual void Wait(Condition & condition) override;
		
		virtual void Launch(Script & script) override;
		
		void Run();
		
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
		Script & _script;
		bool _complete;
	};
}
