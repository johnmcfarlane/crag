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

#include "Condition.h"

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
	
	// blocking engine call
	template <typename RETURN_TYPE, typename ENGINE>
	typename core::raw_type<RETURN_TYPE>::type AppletInterface::Poll(RETURN_TYPE (ENGINE::* function)() const)
	{
		typedef typename core::raw_type<RETURN_TYPE>::type ValueType;
		
		// local stack storage for result
		ValueType result;
		
		// flag to say we're complete;
		PollCondition condition;
		
		// ask daemon to send the poll command to the engine
		ENGINE::Daemon::Poll(function, condition, result);
		
		// result tests the flag upon which applet continuation is condition
		Wait(condition);
		
		// at which time, the result is valid
		return result;
	}
}
