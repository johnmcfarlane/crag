//
//  AppletInterface_Poll.h
//  crag
//
//  Created by John McFarlane on 2012-04-12.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "AppletInterface.h"

#include "FunctorCondition.h"

#include "smp/Message.h"


namespace applet
{
	////////////////////////////////////////////////////////////////////////////////
	// AppletInterface member definitions
	
	// condition type used by AppletInterface::Poll
	class AppletInterface::PollCondition : public Condition
	{
	public:
		~PollCondition()
		{
			ASSERT(_complete);
		}
		operator bool & ()
		{
			return _complete;
		}
	private:
		virtual bool operator() (bool hurry) final
		{
			return _complete;
		}
		
		// variables
		bool _complete = false;
	};
	
	// blocking functor call
	template <typename FUNCTOR>
	void AppletInterface::WaitFor(FUNCTOR functor)
	{
		FunctorCondition<FUNCTOR> condition(functor);
		Wait(condition);
	}

	// blocking engine call
	template <typename ENGINE, typename RETURN_TYPE, typename... PARAMETERS>
	typename core::raw_type<RETURN_TYPE>::type AppletInterface::Poll(RETURN_TYPE (ENGINE::* function)(PARAMETERS const & ...), PARAMETERS const &... parameters)
	{
		typedef typename core::raw_type<RETURN_TYPE>::type ValueType;
		typedef RETURN_TYPE (ENGINE::* FunctionType)(PARAMETERS const & ...);
		
		// local stack storage for result
		ValueType result;
		
		// flag to say we're complete;
		PollCondition condition;
		
		// ask daemon to send the poll command to the engine
		ENGINE::Daemon::template Poll(result, condition, function, parameters...);
		
		// result tests the flag upon which applet continuation is condition
		Wait(condition);
		
		// at which time, the result is valid
		return result;
	}
	
	// blocking engine call
	template <typename ENGINE, typename RETURN_TYPE, typename... PARAMETERS>
	typename core::raw_type<RETURN_TYPE>::type AppletInterface::Poll(RETURN_TYPE (ENGINE::* function)(PARAMETERS const & ...) const, PARAMETERS const &... parameters)
	{
		typedef typename core::raw_type<RETURN_TYPE>::type ValueType;
		typedef RETURN_TYPE (ENGINE::* FunctionType)(PARAMETERS const & ...) const;
		
		// local stack storage for result
		ValueType result;
		
		// flag to say we're complete;
		PollCondition condition;
		
		// ask daemon to send the poll command to the engine
		ENGINE::Daemon::template Poll(result, condition, function, parameters...);
		
		// result tests the flag upon which applet continuation is condition
		Wait(condition);
		
		// at which time, the result is valid
		return result;
	}
}
