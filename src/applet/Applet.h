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

#include "ipc/Fiber.h"
#include "ipc/ObjectBase.h"

namespace applet
{
	// Base class for applets, which are run in fibers.
	// If your applet is sufficiently complex that it deserves its own class,
	// derive that class from this one. Alternatively, specialize the Applet class.
	class Applet : public ipc::ObjectBase<Applet, Engine>, public AppletInterface
	{
		////////////////////////////////////////////////////////////////////////////////
		// types

		typedef ipc::ObjectBase<Applet, Engine> super;
	public:

		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Applet(Engine & engine, char const * name, std::size_t stack_size, LaunchFunction const & function);
		~Applet();
		
		char const * GetName() const;
		
		// true iff the applet has not yet returned
		bool IsRunning() const;
		
		// the condition which must test true before the applet can continue
		const Condition & GetCondition() const;
		
		// continue execution
		void Continue();

		void SetQuitFlag();
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Applet);
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
		
		ipc::Fiber _fiber;
		LaunchFunction _function;
		Condition _condition;
		bool _quit_flag;
	};
}
