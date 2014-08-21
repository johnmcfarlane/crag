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
	DECLARE_CLASS_HANDLE(Applet);	// applet::AppletHandle

	// condition on which to wake from a WaitFor call
	typedef std::function<void (AppletInterface &)> LaunchFunction;

	// Base class for applets, which are run in fibers.
	// If your applet is sufficiently complex that it deserves its own class,
	// derive that class from this one. Alternatively, specialize the Applet class.
	class Applet final : public ipc::ObjectBase<Applet, Engine>, public AppletInterface
	{
		////////////////////////////////////////////////////////////////////////////////
		// types

		using super = ipc::ObjectBase<Applet, Engine>;
	public:

		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Applet(Engine & engine, char const * name, std::size_t stack_size, LaunchFunction const & function);
		~Applet();
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Applet);

		// true iff the applet has not yet returned
		bool IsRunning() const;
		
		// the absolute time when the Applet wishes to be continued
		core::Time GetWakeTime() const;
		
		// continue execution
		void Continue();

		// applet should return from its launch function if quit flag is set
		void SetQuitFlag();

		// AppletInterface overrides
		bool GetQuitFlag() const override;
		
		Engine & GetEngine() const override;

		char const * GetName() const override;
		
		bool Sleep(core::Time duration) override;

	private:
		// called on fiber startup
		static void OnLaunch (void *);

		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		ipc::Fiber _fiber;
		LaunchFunction _function;
		core::Time _wake_time;
		bool _quit_flag;
	};
}
