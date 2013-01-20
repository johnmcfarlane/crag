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

namespace applet
{
	////////////////////////////////////////////////////////////////////////////////
	// AppletInterface member definitions
	
	template <typename FUNCTOR>
	void AppletInterface::WaitFor(FUNCTOR functor)
	{
		Condition condition(functor);
		WaitFor(condition);
	}
	
	template <typename RESULT_TYPE>
	void AppletInterface::WaitFor(smp::Future<RESULT_TYPE> const & future)
	{
		WaitFor([& future] () -> bool {
			return future.IsComplete();
		});
		
		ASSERT(future.IsComplete());
	}
	
	template <typename ENGINE, typename RESULT_TYPE, typename FUNCTION_TYPE>
	void AppletInterface::Call(smp::Future<RESULT_TYPE> & future, FUNCTION_TYPE const & function)
	{
		ASSERT(future.IsPending());

		ENGINE::Daemon::Call([& future, & function] (ENGINE & engine) {
			ASSERT(future.IsPending());
			future.OnSuccess(function(engine));
		});
	}
	
	template <typename ENGINE, typename RESULT_TYPE, typename OBJECT_TYPE, typename FUNCTION_TYPE>
	void AppletInterface::Call(smp::Future<RESULT_TYPE> & future, smp::Handle<OBJECT_TYPE> object, FUNCTION_TYPE const & function)
	{
		ASSERT(future.IsPending());
		
		ENGINE::Daemon::Call([& future, object, & function] (ENGINE & engine) {
			ASSERT(future.IsPending());
			
			auto * base = engine.GetObject(object.GetUid());
			if (base == nullptr) {
				future.OnFailure();
				return;
			}
			
			auto & derived = static_cast<OBJECT_TYPE &>(* base);
			future.OnSuccess(function(derived));
		});
	}

	template <typename ENGINE, typename RESULT_TYPE, typename FUNCTION_TYPE>
	RESULT_TYPE AppletInterface::Get(FUNCTION_TYPE const & function)
	{
		smp::Future<RESULT_TYPE> future;
		Call<ENGINE, RESULT_TYPE, FUNCTION_TYPE>(future, function);
		
		WaitFor(future);
		
		return future.Get();
	}

	template <typename ENGINE, typename RESULT_TYPE, typename OBJECT_TYPE, typename FUNCTION_TYPE>
	smp::Future<RESULT_TYPE> AppletInterface::Get(smp::Handle<OBJECT_TYPE> object, FUNCTION_TYPE const & function)
	{
		smp::Future<RESULT_TYPE> future;
		Call<ENGINE, RESULT_TYPE, OBJECT_TYPE, FUNCTION_TYPE>(future, object, function);
		
		WaitFor(future);
		
		return future;
	}
}
