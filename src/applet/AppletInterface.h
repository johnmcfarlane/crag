//
//  AppletInterface.h
//  crag
//
//  Created by John McFarlane on 2012-02-10.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

namespace applet
{
	// forward-declare
	class Engine;

	// Interface to the Applet class;
	// This interface is passed to a Applet's function object.
	class AppletInterface
	{
	public:
		// functions
		virtual ~AppletInterface() { }
		
		virtual char const * GetName() const = 0;
		
		// applet thread quit flag get/setter
		virtual bool GetQuitFlag() const = 0;

		// pause execution in various ways
		virtual void Sleep(core::Time duration) = 0;
		virtual void WaitFor(Condition & condition) = 0;
		
		// blocks until the given functor returns true
		template <typename FUNCTOR>
		void WaitFor(FUNCTOR functor);
		
		// blocks until the given future is fulfilled
		template <typename RESULT_TYPE>
		void WaitFor(ipc::Future<RESULT_TYPE> const & future);
		
		// non-blocking call to engine on separate thread
		template <typename ENGINE, typename RESULT_TYPE, typename FUNCTION_TYPE>
		void Call(ipc::Future<RESULT_TYPE> & future, FUNCTION_TYPE const & function);
		
		// non-blocking call to object on separate thread
		template <typename ENGINE, typename RESULT_TYPE, typename OBJECT_TYPE, typename FUNCTION_TYPE>
		void Call(ipc::Future<RESULT_TYPE> & future, ipc::Handle<OBJECT_TYPE> object, FUNCTION_TYPE const & function);
		
		// blocking call to engine on separate thread
		template <typename ENGINE, typename RESULT_TYPE, typename FUNCTION_TYPE>
		RESULT_TYPE Get(FUNCTION_TYPE const & function);
		
		// blocking call to object on separate thread
		template <typename ENGINE, typename RESULT_TYPE, typename OBJECT_TYPE, typename FUNCTION_TYPE>
		ipc::Future<RESULT_TYPE> Get(ipc::Handle<OBJECT_TYPE> object, FUNCTION_TYPE const & function);

		template <typename FUNCTION_TYPE>
		void Launch(char const * name, std::size_t stack_size, FUNCTION_TYPE const & function);

	private:
		virtual Engine & GetEngine() = 0;
	};
}
