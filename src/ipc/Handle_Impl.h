//
//  Handle_Impl.h
//  crag
//
//  Created by John McFarlane on 2012-05-04.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Handle.h"

////////////////////////////////////////////////////////////////////////////////
// Handle friend definitions

namespace std
{
	template <typename TYPE>
	struct hash <ipc::Handle<TYPE>>
	{
		size_t operator() (ipc::Handle<TYPE> handle) const
		{
			return hash <ipc::Uid> () (handle._uid);
		}
	};
}

namespace ipc
{
	////////////////////////////////////////////////////////////////////////////////
	// Handle member definitions
	
	template <typename TYPE>
	template <typename BASE_TYPE>
	Handle<TYPE>::operator Handle<BASE_TYPE> () const
	{
		static_assert(std::is_base_of<BASE_TYPE, TYPE>::value, "invalid cast");
		return Handle<BASE_TYPE>::CreateFromUid(_uid);
	}
	
	template <typename TYPE>
	template <typename ... PARAMETERS>
	Handle<TYPE> Handle<TYPE>::Create(PARAMETERS && ... parameters)
	{
		Handle creation;
		creation.CreateObject(std::forward<PARAMETERS>(parameters) ...); 
		return creation;
	}

	template <typename TYPE>
	template <typename ... PARAMETERS>
	void Handle<TYPE>::CreateObject(PARAMETERS ... parameters)
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;

		Release();
		Handle handle(Uid::Create());
		Daemon::Call([handle, parameters ...] (Engine & engine) {
			engine.template CreateObject<ObjectType, PARAMETERS ...>(handle, parameters ...);
		});

		* this = handle;
	}
	
	// Tells simulation to release the object.
	template <typename TYPE>
	void Handle<TYPE>::Release()
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;
		
		// If not already released,
		if (_uid.IsInitialized())
		{
			// set message.
			auto handle = * this;
			_uid = Uid();
			
			Daemon::Call([handle] (Engine & engine) {
				engine.ReleaseObject(handle);
			});
		}
	}

	// calls the given function with a reference to the handle's object
	// (unless the handle is not valid when call is put through) 
	template <typename TYPE>
	template <typename FUNCTION_TYPE>
	void Handle<TYPE>::Call(FUNCTION_TYPE function) const
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;
		
		ASSERT(_uid.IsInitialized());

		auto handle = * this;
		Daemon::Call([function, handle] (Engine & engine) {
			auto base = engine.GetObject(handle);
			if (base != nullptr)
			{
				auto & derived = core::StaticCast<ObjectType>(* base);
				function(derived);
			}
		});
	}
	
	// calls the given function with a pointer to the handle's object
	// (or nullptr if the handle does not reference a valid object)
	template <typename TYPE>
	template <typename FUNCTION_TYPE>
	void Handle<TYPE>::CallPtr(FUNCTION_TYPE function) const
	{
		ASSERT(_uid.IsInitialized());

		auto handle = * this;
		ObjectType::Daemon::Call([function, handle] (typename ObjectType::Engine & engine) {
			auto base = engine.GetObject(handle);
			auto derived = core::StaticCast<ObjectType>(base);
			function(derived);
		});
	}
	
	// calls the given function with a reference to the handle's object
	// and stores result in given future
	template <typename TYPE>
	template <typename VALUE_TYPE, typename FUNCTION_TYPE>
	void Handle<TYPE>::Call(Future<VALUE_TYPE> & future, FUNCTION_TYPE function) const
	{
		ASSERT(_uid.IsInitialized());
		ASSERT(future.IsPendind());
		
		auto handle = * this;
		ObjectType::Daemon::Call([& future, function, handle] (typename ObjectType::Engine & engine) {
			ASSERT(future.IsPendind());
			
			auto base = engine.GetObject(handle);
			if (base == nullptr) {
				future.OnFailure();
				return;
			}
			
			auto & derived = core::StaticCast<ObjectType &>(* base);
			future.OnSuccess(function(derived));
		});
	}
}
