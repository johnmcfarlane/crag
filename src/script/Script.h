//
//  Script.h
//  crag
//
//  Created by John McFarlane on 2012-04-06.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "ScriptBase.h"


namespace script
{
	// A templated script class. 
	// Handy for providing a function to be launched in its own fiber.
	template <typename FUNCTOR = void (*)(FiberInterface &)>
	class Script : public ScriptBase
	{
	public:
		// types
		typedef FUNCTOR Functor;
		
		// functions
		Script(Functor functor)
		: _functor(functor)
		{
		}
		
	private:
		// ScriptBase overrides
		virtual void operator() (FiberInterface & script_interface) override
		{
			_functor(script_interface);
		}

		// variables
		Functor _functor;
	};
	
	typedef smp::Handle< Script<> > ScriptHandle;
}
