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
#include "Future.h"

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
	template <typename ENGINE, typename RETURN_TYPE, typename... PARAMETERS>
	typename core::raw_type<RETURN_TYPE>::type AppletInterface::Call(RETURN_TYPE (ENGINE::* function)(PARAMETERS const & ...), PARAMETERS const &... parameters)
	{
		typedef typename core::raw_type<RETURN_TYPE>::type ValueType;
		
		// local stack storage for result
		Future<ValueType> future(* this, function, parameters...);
		
		// at which time, the result is valid
		return future.get();
	}
	
	// blocking engine call to non-const function
	template <typename ENGINE, typename RETURN_TYPE, typename... PARAMETERS>
	typename core::raw_type<RETURN_TYPE>::type AppletInterface::Call(RETURN_TYPE (ENGINE::* function)(PARAMETERS const & ...) const, PARAMETERS const &... parameters)
	{
		typedef typename core::raw_type<RETURN_TYPE>::type ValueType;
		
		// local stack storage for result
		Future<ValueType> future(* this, function, parameters...);
		
		// at which time, the result is valid
		return future.get();
	}
}
