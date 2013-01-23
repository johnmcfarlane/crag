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

#include "smp/Object.h"


namespace smp 
{
	class Fiber;
}

namespace applet
{
	// Base class for applets, which are run in fibers.
	// If your applet is sufficiently complex that it deserves its own class,
	// derive that class from this one. Alternatively, specialize the Applet class.
	class AppletBase : public smp::Object<AppletBase, Engine>, public AppletInterface
	{
		typedef smp::Object<AppletBase, Engine> super;
	public:
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		AppletBase(Init const & init, char const * name, std::size_t stack_size);
		~AppletBase();
		
		char const * GetName() const;
		
		// true iff the applet has not yet returned
		bool IsRunning() const;
		
		// the condition which must test true before the applet can continue
		const Condition & GetCondition() const;
		
		// continue execution
		void Continue();

		void SetQuitFlag();
		
#if defined(VERIFY)
		void Verify() const;
#endif
	private:
		
		// AppletInterface overrides
		virtual bool GetQuitFlag() const override;
		
		virtual void Sleep(core::Time duration) override;
		virtual void WaitFor(Condition & condition) override;

		virtual Engine & GetEngine() override;

		// overridden by the concrete applet class
		virtual void operator() (AppletInterface & applet_interface) = 0;

		// called on fiber startup
		static void OnLaunch (void *);

		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		DEFINE_INTRUSIVE_LIST(AppletBase, List);
		smp::Fiber & _fiber;
		Condition _condition;
		bool _quit_flag;
	};
}
