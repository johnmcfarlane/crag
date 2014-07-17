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
	ostream & 
	operator << (ostream & out, ::ipc::Handle<TYPE> const & handle)
	{
		return out << handle._uid;
	}

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
	
#if defined(WIN32)
	template <typename TYPE>
	template <typename ... PARAMETERS>
	Handle<TYPE> Handle<TYPE>::Create(PARAMETERS ... parameters)
	{
		Handle creation;
		creation.CreateObject(parameters ...); 
		return creation;
	}
#else
	template <typename TYPE>
	template <typename ... PARAMETERS>
	Handle<TYPE> Handle<TYPE>::Create(PARAMETERS && ... parameters)
	{
		Handle creation;
		creation.CreateObject(std::forward<PARAMETERS>(parameters) ...); 
		return creation;
	}
#endif

#if defined(WIN32) || ! defined(__clang__)
	template <typename TYPE>
	void Handle<TYPE>::CreateObject()
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;

		Release();
		Handle handle(Uid::Create());
		Daemon::Call([handle] (Engine & engine) {
			engine.template CreateObject<ObjectType>(handle);
		});

		* this = handle;
	}

	template <typename TYPE>
	template <typename PARAMETER1>
	void Handle<TYPE>::CreateObject(PARAMETER1 parameter1)
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;

		Release();
		Handle handle(Uid::Create());
		Daemon::Call([handle, parameter1] (Engine & engine) {
			engine.template CreateObject<ObjectType, PARAMETER1>(handle, parameter1);
		});

		* this = handle;
	}

	template <typename TYPE>
	template <typename PARAMETER1, typename PARAMETER2>
	void Handle<TYPE>::CreateObject(PARAMETER1 parameter1, PARAMETER2 parameter2)
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;

		Release();
		Handle handle(Uid::Create());
		Daemon::Call([handle, parameter1, parameter2] (Engine & engine) {
			engine.template CreateObject<ObjectType, PARAMETER1, PARAMETER2>(handle, parameter1, parameter2);
		});

		* this = handle;
	}

	template <typename TYPE>
	template <typename PARAMETER1, typename PARAMETER2, typename PARAMETER3>
	void Handle<TYPE>::CreateObject(PARAMETER1 parameter1, PARAMETER2 parameter2, PARAMETER3 parameter3)
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;

		Release();
		Handle handle(Uid::Create());
		Daemon::Call([handle, parameter1, parameter2, parameter3] (Engine & engine) {
			engine.template CreateObject<ObjectType, PARAMETER1, PARAMETER2, PARAMETER3>(handle, parameter1, parameter2, parameter3);
		});

		* this = handle;
	}

	template <typename TYPE>
	template <typename PARAMETER1, typename PARAMETER2, typename PARAMETER3, typename PARAMETER4>
	void Handle<TYPE>::CreateObject(PARAMETER1 parameter1, PARAMETER2 parameter2, PARAMETER3 parameter3, PARAMETER4 parameter4)
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;

		Release();
		Handle handle(Uid::Create());
		Daemon::Call([handle, parameter1, parameter2, parameter3, parameter4] (Engine & engine) {
			engine.template CreateObject<ObjectType, PARAMETER1, PARAMETER2, PARAMETER3, PARAMETER4>(handle, parameter1, parameter2, parameter3, parameter4);
		});

		* this = handle;
	}

	template <typename TYPE>
	template <typename PARAMETER1, typename PARAMETER2, typename PARAMETER3, typename PARAMETER4, typename PARAMETER5>
	void Handle<TYPE>::CreateObject(PARAMETER1 parameter1, PARAMETER2 parameter2, PARAMETER3 parameter3, PARAMETER4 parameter4, PARAMETER5 parameter5)
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;

		Release();
		Handle handle(Uid::Create());
		Daemon::Call([handle, parameter1, parameter2, parameter3, parameter4, parameter5] (Engine & engine) {
			engine.template CreateObject<ObjectType, PARAMETER1, PARAMETER2, PARAMETER3, PARAMETER4, PARAMETER5>(handle, parameter1, parameter2, parameter3, parameter4, parameter5);
		});

		* this = handle;
	}
#else
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
#endif
	
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
