//
//  Future.h
//  crag
//
//  Created by John McFarlane on 2012-04-24.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "PollCondition.h"

namespace applet
{
	// A deferred result from a cross-thread function call.
	// This is a good way to call a function which returns a value
	// without having to wait for the result straight away.
	template <typename VALUE>
	class Future : public Condition
	{
	public:
		// performs thread-safe call to the given function using the given AppletInterface
		template <typename ENGINE, typename RETURN_TYPE, typename... PARAMETERS>
		Future(AppletInterface & applet_interface, RETURN_TYPE (ENGINE::* function)(PARAMETERS const & ...) const, PARAMETERS const &... parameters)
		: _applet_interface(applet_interface)
		, _complete(false)
		{
			typedef typename core::raw_type<RETURN_TYPE>::type ValueType;
			typedef RETURN_TYPE (ENGINE::* FunctionType)(PARAMETERS const & ...);
			
			// ask daemon to send the poll command to the engine
			ENGINE::Daemon::template Poll(_value, _complete, function, parameters...);
		}
		
		~Future()
		{
			ASSERT(_complete);
		}
		
		// returns true iff the result
		bool valid() const
		{
			return _complete;
		}
		
		// blocks until valid and then returns result of function call given in c'tor
		VALUE & get()
		{
			wait();
			return _value;
		}
		
		// blocks until valid
		void wait()
		{
			if (! _complete) 
			{
				PollCondition condition(_complete);
				_applet_interface.Wait(condition);
				ASSERT(_complete);
			}
		}
	private:
		virtual bool operator() (bool hurry) override
		{
			return _complete;
		}
		
		// variables
		VALUE _value;
		AppletInterface & _applet_interface;
		bool _complete;
	};
}
