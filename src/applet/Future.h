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
		// performs thread-safe call to the given non-const ENGINE function using the given AppletInterface
		template <typename ENGINE, typename RETURN_TYPE, typename... PARAMETERS>
		Future(AppletInterface & applet_interface, RETURN_TYPE (ENGINE::* function)(PARAMETERS const & ...) const, PARAMETERS const &... parameters)
		: _applet_interface(applet_interface)
		, _status(smp::pending)
		{
			typedef typename core::raw_type<RETURN_TYPE>::type ValueType;
			typedef RETURN_TYPE (ENGINE::* FunctionType)(PARAMETERS const & ...);
			
			// ask daemon to send the poll command to the engine
			ENGINE::Daemon::template Poll(_value, _status, function, parameters...);
		}
		
		// performs thread-safe call to the given const ENGINE function using the given AppletInterface
		template <typename ENGINE, typename RETURN_TYPE, typename... PARAMETERS>
		Future(AppletInterface & applet_interface, RETURN_TYPE (ENGINE::* function)(PARAMETERS const & ...), PARAMETERS const &... parameters)
		: _applet_interface(applet_interface)
		, _status(smp::pending)
		{
			typedef typename core::raw_type<RETURN_TYPE>::type ValueType;
			typedef RETURN_TYPE (ENGINE::* FunctionType)(PARAMETERS const & ...);
			
			// ask daemon to send the poll command to the engine
			ENGINE::Daemon::template Poll(_value, _status, function, parameters...);
		}
		
		// performs thread-safe call to the given OBJECT function using the given AppletInterface
		template <typename OBJECT, typename RETURN_TYPE, typename... PARAMETERS>
		Future(AppletInterface & applet_interface, smp::Handle<OBJECT> const & handle, RETURN_TYPE (OBJECT::* function)(PARAMETERS const & ...) const, PARAMETERS const &... parameters)
		: _applet_interface(applet_interface)
		, _status(smp::pending)
		{
			typedef typename core::raw_type<RETURN_TYPE>::type ValueType;
			typedef RETURN_TYPE (OBJECT::* FunctionType)(PARAMETERS const & ...);
			
			// ask daemon to send the poll command to the engine
			handle.Poll(_value, _status, function, parameters...);
		}
		
		~Future()
		{
			ASSERT(_status == smp::complete || _status == smp::failed);
		}
		
		// returns true iff the result
		bool valid() const
		{
			return _status == smp::complete;
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
			if (_status == smp::pending) 
			{
				PollCondition condition(_status);
				_applet_interface.Wait(condition);
				ASSERT(_status == smp::complete || _status == smp::failed);
			}
		}
	private:
		virtual bool operator() (bool hurry) override
		{
			return _status != smp::pending;
		}
		
		// variables
		VALUE _value;
		AppletInterface & _applet_interface;
		smp::PollStatus _status;
	};
}
