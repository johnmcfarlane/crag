//
//  AppletBase.h
//  crag
//
//  Created by John McFarlane on 2012-03-07.
//  Copyright (c) 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "AppletInterface.h"

#include "smp/ObjectBase.h"


namespace smp 
{
	class Fiber;
	class FiberInterface;
}

namespace applet
{
	// forward-declarations
	class Engine;
	
	// Base class for applets, which are run in fibers.
	// If your applet is sufficiently complex that it deserves its own class,
	// derive that class from this one. Alternatively, specialize the Applet class.
	class AppletBase : public smp::ObjectBase<AppletBase, Engine>, public AppletInterface
	{
		typedef smp::ObjectBase<AppletBase, Engine> super;
	public:
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		AppletBase(Init const & init);
		~AppletBase();
		
		// true iff the applet has not yet returned
		bool IsRunning() const;
		
		// the condition which must test true before the applet can continue
		const Condition & GetCondition() const;
		
		// continue execution
		void Continue();

		void SetQuitFlag();
	private:
		
		// AppletInterface overrides
		virtual bool GetQuitFlag() const override;
		
		virtual void Yield() override;
		virtual void Sleep(core::Time duration) override;
		virtual void WaitFor(Condition & condition) override;

		// overridden by the concrete applet class
		virtual void operator() (AppletInterface & applet_interface) = 0;

		// called on fiber startup
		static void OnLaunch (void *);

		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		smp::Fiber & _fiber;
		Condition _condition;
		bool _quit_flag;
		bool _finished_flag;
	};
}
