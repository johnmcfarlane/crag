//
//  Applet.h
//  crag
//
//  Created by John McFarlane on 2012-03-07.
//  Copyright (c) 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "AppletInterface.h"

#include "smp/Fiber.h"
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
	class Applet : public smp::Object<Applet, Engine>, public AppletInterface
	{
		////////////////////////////////////////////////////////////////////////////////
		// types

		typedef smp::Object<Applet, Engine> super;
	public:

		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Applet(Init const & init, char const * name, std::size_t stack_size, LaunchFunction const & function);
		~Applet();
		
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

		// called on fiber startup
		static void OnLaunch (void *);

		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		smp::Fiber _fiber;
		LaunchFunction _function;
		Condition _condition;
		bool _quit_flag;
	};
}
