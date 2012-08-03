//
//  AppletInterface_Impl.h
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
	
	// blocking functor call
	template <typename FUNCTOR>
	void AppletInterface::WaitFor(FUNCTOR functor)
	{
		FunctorCondition<FUNCTOR> condition(functor);
		Wait(condition);
	}

	// blocking engine call to const function
	template <typename ENGINE, typename RESULT_TYPE, typename FUNCTION_TYPE>
	RESULT_TYPE AppletInterface::Poll(FUNCTION_TYPE const & function)
	{
		RESULT_TYPE result;
		smp::PollStatus status = smp::pending;
		ENGINE::Daemon::Call([& function, & result, & status] (ENGINE & engine) {
			ASSERT(status == smp::pending);
			result = function(engine);
			std::atomic_thread_fence(std::memory_order_seq_cst);
			status = smp::complete;
		});
		
		WaitFor([& status] () -> bool {
			return status != smp::pending;
		});
		
		ASSERT(status == smp::complete);
		return result;
	}
	
	// non-blocking caller
	template <typename ENGINE, typename FUNCTION_TYPE>
	void AppletInterface::Call(FUNCTION_TYPE const & function)
	{
		ENGINE::Daemon::Call(function);
	}
}
