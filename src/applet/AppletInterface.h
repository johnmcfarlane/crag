//
//  AppletInterface.h
//  crag
//
//  Created by John McFarlane on 2012-02-10.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace applet
{
	// forward-declare
	class Condition;
	class AppletBase;
	
	// Interface to the Applet class;
	// This interface is passed to a Applet's function object.
	class AppletInterface
	{
	public:
		// functions
		virtual ~AppletInterface() { }
		
		// applet thread quit flag get/setter
		virtual bool GetQuitFlag() const = 0;

		// pause execution in various ways
		virtual void Yield() = 0;
		virtual void Sleep(Time duration) = 0;
		virtual void Wait(Condition & condition) = 0;
		
		template <typename FUNCTOR>
		void WaitFor(FUNCTOR functor);
		
		// Calls a cross-thread engine function 
		// and blocks until it can return the result. 
		// (Defined in AppletInterface_Impl.h.)
		template <typename ENGINE, typename RESULT_TYPE, typename FUNCTION_TYPE>
		RESULT_TYPE Poll(FUNCTION_TYPE const & function);
		
		// non-blocking caller
		template <typename ENGINE, typename FUNCTION_TYPE>
		void Call(FUNCTION_TYPE const & function);
	};
}
