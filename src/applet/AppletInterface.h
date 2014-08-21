//
//  AppletInterface.h
//  crag
//
//  Created by John McFarlane on 2012-02-10.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "ipc/Handle.h"

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
		
		// applet thread quit flag get/setter
		virtual bool GetQuitFlag() const = 0;
		
		virtual Engine & GetEngine() const = 0;

		virtual char const * GetName() const = 0;
		
		// pause execution; returns false if quit flag is set
		virtual bool Sleep(core::Time duration) = 0;
		
		// non-blocking call to engine on separate thread
		template <typename ENGINE, typename RESULT_TYPE, typename FUNCTION_TYPE>
		void Call(ipc::Future<RESULT_TYPE> & future, FUNCTION_TYPE const & function);
		
		// non-blocking call to object on separate thread
		template <typename ENGINE, typename RESULT_TYPE, typename OBJECT_TYPE, typename FUNCTION_TYPE>
		void Call(ipc::Future<RESULT_TYPE> & future, ipc::Handle<OBJECT_TYPE> object, FUNCTION_TYPE const & function);
		
		// blocking call to engine on separate thread
		template <typename ENGINE, typename RESULT_TYPE, typename FUNCTION_TYPE>
		RESULT_TYPE Get(FUNCTION_TYPE const & function);
		
		template <typename FUNCTION_TYPE>
		void Launch(char const * name, std::size_t stack_size, FUNCTION_TYPE const & function);
	};
}
