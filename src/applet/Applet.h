//
//  Applet.h
//  crag
//
//  Created by John McFarlane on 2012-04-06.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "AppletBase.h"


namespace applet
{
	// A templated applet class. 
	// Handy for providing a function to be launched in its own fiber.
	template <typename FUNCTOR = void (*)(AppletInterface &)>
	class Applet : public AppletBase
	{
	public:
		// types
		typedef FUNCTOR Functor;
		
		// functions
		Applet(Init const & init, Functor functor)
		: AppletBase(init)
		, _functor(functor)
		{
		}
		
	private:
		// AppletBase overrides
		virtual void operator() (AppletInterface & applet_interface) override
		{
			_functor(applet_interface);
		}

		// variables
		Functor _functor;
	};
}
