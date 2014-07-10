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

namespace ipc
{
	////////////////////////////////////////////////////////////////////////////////
	// Handle member definitions
	
	template <typename TYPE>
	template <typename BASE_TYPE>
	Handle<TYPE>::operator Handle<BASE_TYPE> () const
	{
		static_assert(std::is_base_of<BASE_TYPE, TYPE>::value, "invalid cast");
		return Handle <BASE_TYPE>(_uid);
	}
	
#if defined(WIN32)
	template <typename TYPE>
	template <typename ... PARAMETERS>
	Handle<TYPE> Handle<TYPE>::CreateHandle(PARAMETERS ... parameters)
	{
		Handle creation;
		creation.Create(parameters ...); 
		return creation;
	}
#else
	template <typename TYPE>
	template <typename ... PARAMETERS>
	Handle<TYPE> Handle<TYPE>::CreateHandle(PARAMETERS && ... parameters)
	{
		Handle creation;
		creation.Create(std::forward<PARAMETERS>(parameters) ...); 
		return creation;
	}
#endif

#if defined(WIN32) || ! defined(__clang__)
	template <typename TYPE>
	void Handle<TYPE>::Create()
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;

		Release();
		Uid uid = Uid::Create();
		Daemon::Call([uid] (Engine & engine) {
			engine.template CreateObject<ObjectType>(uid);
		});

		_uid = uid;
	}

	template <typename TYPE>
	template <typename PARAMETER1>
	void Handle<TYPE>::Create(PARAMETER1 parameter1)
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;

		Release();
		Uid uid = Uid::Create();
		Daemon::Call([uid, parameter1] (Engine & engine) {
			engine.template CreateObject<ObjectType, PARAMETER1>(uid, parameter1);
		});

		_uid = uid;
	}

	template <typename TYPE>
	template <typename PARAMETER1, typename PARAMETER2>
	void Handle<TYPE>::Create(PARAMETER1 parameter1, PARAMETER2 parameter2)
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;

		Release();
		Uid uid = Uid::Create();
		Daemon::Call([uid, parameter1, parameter2] (Engine & engine) {
			engine.template CreateObject<ObjectType, PARAMETER1, PARAMETER2>(uid, parameter1, parameter2);
		});

		_uid = uid;
	}

	template <typename TYPE>
	template <typename PARAMETER1, typename PARAMETER2, typename PARAMETER3>
	void Handle<TYPE>::Create(PARAMETER1 parameter1, PARAMETER2 parameter2, PARAMETER3 parameter3)
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;

		Release();
		Uid uid = Uid::Create();
		Daemon::Call([uid, parameter1, parameter2, parameter3] (Engine & engine) {
			engine.template CreateObject<ObjectType, PARAMETER1, PARAMETER2, PARAMETER3>(uid, parameter1, parameter2, parameter3);
		});

		_uid = uid;
	}

	template <typename TYPE>
	template <typename PARAMETER1, typename PARAMETER2, typename PARAMETER3, typename PARAMETER4>
	void Handle<TYPE>::Create(PARAMETER1 parameter1, PARAMETER2 parameter2, PARAMETER3 parameter3, PARAMETER4 parameter4)
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;

		Release();
		Uid uid = Uid::Create();
		Daemon::Call([uid, parameter1, parameter2, parameter3, parameter4] (Engine & engine) {
			engine.template CreateObject<ObjectType, PARAMETER1, PARAMETER2, PARAMETER3, PARAMETER4>(uid, parameter1, parameter2, parameter3, parameter4);
		});

		_uid = uid;
	}

	template <typename TYPE>
	template <typename PARAMETER1, typename PARAMETER2, typename PARAMETER3, typename PARAMETER4, typename PARAMETER5>
	void Handle<TYPE>::Create(PARAMETER1 parameter1, PARAMETER2 parameter2, PARAMETER3 parameter3, PARAMETER4 parameter4, PARAMETER5 parameter5)
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;

		Release();
		Uid uid = Uid::Create();
		Daemon::Call([uid, parameter1, parameter2, parameter3, parameter4, parameter5] (Engine & engine) {
			engine.template CreateObject<ObjectType, PARAMETER1, PARAMETER2, PARAMETER3, PARAMETER4, PARAMETER5>(uid, parameter1, parameter2, parameter3, parameter4, parameter5);
		});

		_uid = uid;
	}
#else
	template <typename TYPE>
	template <typename ... PARAMETERS>
	void Handle<TYPE>::Create(PARAMETERS ... parameters)
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;

		Release();
		Uid uid(Uid::Create());
		Daemon::Call([uid, parameters ...] (Engine & engine) {
			engine.template CreateObject<ObjectType, PARAMETERS ...>(Handle(uid), parameters ...);
		});

		_uid = uid;
	}
#endif
	
	// Tells simulation to release the object.
	template <typename TYPE>
	void Handle<TYPE>::Release()
	{
		using Daemon = typename ObjectType::DaemonType;
		using Engine = typename ObjectType::EngineType;
		
		// If not already released,
		if (_uid)
		{
			// set message.
			Uid uid = _uid;
			Daemon::Call([uid] (Engine & engine) {
				engine.ReleaseObject(Handle(uid));
			});
			_uid = Uid();
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
		
		ASSERT(_uid);

		auto handle = * this;
		Daemon::Call([function, handle] (Engine & engine) {
			auto base = engine.GetObject(handle);
			if (base != nullptr)
			{
				auto& derived = core::StaticCast<ObjectType>(* base);
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
		ASSERT(_uid);

		auto uid = _uid;
		ObjectType::Daemon::Call([function, uid] (typename ObjectType::Engine & engine) {
			auto base = engine.GetObject(uid);
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
		ASSERT(_uid);
		ASSERT(future.IsPendind());
		
		auto uid = _uid;
		ObjectType::Daemon::Call([& future, function, uid] (typename ObjectType::Engine & engine) {
			ASSERT(future.IsPendind());
			
			auto base = engine.GetObject(uid);
			if (base == nullptr) {
				future.OnFailure();
				return;
			}
			
			auto & derived = core::StaticCast<ObjectType &>(* base);
			future.OnSuccess(function(derived));
		});
	}
}
