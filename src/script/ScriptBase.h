//
//  ScriptBase.h
//  crag
//
//  Created by John McFarlane on 2012-03-07.
//  Copyright (c) 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "FiberInterface.h"

#include "smp/ObjectBase.h"


namespace smp 
{
	class Fiber;
	class FiberInterface;
}

namespace script
{
	// forward-declarations
	class Engine;
	
	// Base class for scripts, which are run in fibers.
	class ScriptBase : public smp::ObjectBase<ScriptBase, Engine>, public FiberInterface
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		ScriptBase();
		~ScriptBase();
		
		// true iff the script has not yet returned
		bool IsRunning() const;
		
		// the condition which must test true before the script can continue
		Condition * GetCondition();
		
		// continue execution
		void Continue();

		// called on fiber startup
		virtual void operator() (smp::FiberInterface & fiber);

	private:
		// ScriptInterface overrides
		virtual bool GetQuitFlag() const override;
		virtual void SetQuitFlag() override;
		
		virtual void Yield() override;
		virtual void Sleep(Time duration) override;
		virtual void Wait(Condition & condition) override;
		virtual void Launch(ScriptBase & script) override;

		// overridden by the concrete script class
		virtual void operator() (FiberInterface & script_interface) = 0;

		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		smp::Fiber & _fiber;
		Condition * _condition;
		bool _quit_flag;
	};
}
