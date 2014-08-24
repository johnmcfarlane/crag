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

#include "Engine.h"

namespace applet
{
	////////////////////////////////////////////////////////////////////////////////
	// AppletInterface member definitions
	
	template <typename ENGINE, typename RESULT_TYPE, typename FUNCTION_TYPE>
	void AppletInterface::Call(ipc::Future<RESULT_TYPE> & future, FUNCTION_TYPE const & function)
	{
		ASSERT(future.IsPending());

		ENGINE::Daemon::Call([& future, & function] (ENGINE & engine) {
			ASSERT(future.IsPending());
			future.OnSuccess(function(engine));
		});
	}
	
	template <typename ENGINE, typename RESULT_TYPE, typename OBJECT_TYPE, typename FUNCTION_TYPE>
	void AppletInterface::Call(ipc::Future<RESULT_TYPE> & future, ipc::Handle<OBJECT_TYPE> object, FUNCTION_TYPE const & function)
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
		ipc::Future<RESULT_TYPE> future;
		Call<ENGINE, RESULT_TYPE, FUNCTION_TYPE>(future, function);
		
		do
		{
			Sleep(0);
		}	while (! future.IsComplete());
		
		return future.Get();
	}

	template <typename FUNCTION_TYPE>
	void AppletInterface::Launch(char const * name, std::size_t stack_size, FUNCTION_TYPE const & function)
	{
		AppletHandle::Create(name, stack_size, function);
	}
}
