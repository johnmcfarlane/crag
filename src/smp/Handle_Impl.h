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

namespace smp
{
	////////////////////////////////////////////////////////////////////////////////
	// Handle member definitions
	
	template <typename TYPE>
	Handle<TYPE>::Handle()
	{
	}
	
	template <typename TYPE>
	Handle<TYPE>::Handle(Uid uid)
	: _uid(uid)
	{
	}
	
	template <typename TYPE>
	Handle<TYPE>::Handle(Type & object)
	: _uid(object.GetUid())
	{
	}
	
	template <typename TYPE>
	template <typename BASE_TYPE>
	Handle<TYPE>::operator Handle<BASE_TYPE> & ()
	{
		ASSERT((BASE_TYPE *)((Type *)nullptr) == nullptr);
		return reinterpret_cast<Handle <BASE_TYPE> &>(* this);
	}
	
	template <typename TYPE>
	template <typename BASE_TYPE>
	Handle<TYPE>::operator Handle<BASE_TYPE> const & () const
	{
		ASSERT((BASE_TYPE *)((Type *)nullptr) == nullptr);
		return reinterpret_cast<Handle <BASE_TYPE> const &>(* this);
	}
	
	template <typename TYPE>
	Handle<TYPE>::operator bool () const
	{
		return _uid;
	}
	
	// Returns the UID of the entity being handled.
	template <typename TYPE>
	Uid Handle<TYPE>::GetUid() const
	{
		return _uid;
	}
	
	// Sets the UID of the entity being handled.
	// TODO: ... but then so is this.
	template <typename TYPE>
	void Handle<TYPE>::SetUid(Uid uid)
	{
		_uid = uid;
	}
	
#if defined(__GNUC__)
	template <typename TYPE>
	void Handle<TYPE>::Create()
	{
		typedef typename Type::Daemon Daemon;
		typedef typename Type::Engine Engine;

		Destroy();
		Uid uid = Uid::Create();
		Daemon::Call([uid] (Engine & engine) {
			engine.template CreateObject<Type>(uid);
		});
		SetUid(uid);
	}

	template <typename TYPE>
	template <typename PARAMETER1>
	void Handle<TYPE>::Create(PARAMETER1 parameter1)
	{
		typedef typename Type::Daemon Daemon;
		typedef typename Type::Engine Engine;

		Destroy();
		Uid uid = Uid::Create();
		Daemon::Call([uid, parameter1] (Engine & engine) {
			engine.template CreateObject<Type, PARAMETER1>(uid, parameter1);
		});
		SetUid(uid);
	}

	template <typename TYPE>
	template <typename PARAMETER1, typename PARAMETER2>
	void Handle<TYPE>::Create(PARAMETER1 parameter1, PARAMETER2 parameter2)
	{
		typedef typename Type::Daemon Daemon;
		typedef typename Type::Engine Engine;

		Destroy();
		Uid uid = Uid::Create();
		Daemon::Call([uid, parameter1, parameter2] (Engine & engine) {
			engine.template CreateObject<Type, PARAMETER1, PARAMETER2>(uid, parameter1, parameter2);
		});
		SetUid(uid);
	}

	template <typename TYPE>
	template <typename PARAMETER1, typename PARAMETER2, typename PARAMETER3>
	void Handle<TYPE>::Create(PARAMETER1 parameter1, PARAMETER2 parameter2, PARAMETER3 parameter3)
	{
		typedef typename Type::Daemon Daemon;
		typedef typename Type::Engine Engine;

		Destroy();
		Uid uid = Uid::Create();
		Daemon::Call([uid, parameter1, parameter2, parameter3] (Engine & engine) {
			engine.template CreateObject<Type, PARAMETER1, PARAMETER2>(uid, parameter1, parameter2, parameter3);
		});
		SetUid(uid);
	}
#else
	template <typename TYPE>
	template <typename ... PARAMETERS>
	void Handle<TYPE>::Create(PARAMETERS ... parameters)
	{
		typedef typename Type::Daemon Daemon;
		typedef typename Type::Engine Engine;

		Destroy();
		Uid uid = Uid::Create();
		Daemon::Call([uid, parameters ...] (Engine & engine) {
			engine.template CreateObject<Type, PARAMETERS ...>(uid, parameters ...);
		});
		SetUid(uid);
	}
#endif
	
	// Tells simulation to destroy the object.
	template <typename TYPE>
	void Handle<TYPE>::Destroy()
	{
		typedef typename Type::Daemon Daemon;
		typedef typename Type::Engine Engine;
		
		// If not already destroyed,
		if (_uid)
		{
			// set message.
			Uid uid = _uid;
			Daemon::Call([uid] (Engine & engine) {
				engine.OnRemoveObject(uid);
			});
			_uid = Uid();
		}
	}
	
	// calls the given function with a reference to the handle's object
	template <typename TYPE>
	template <typename FUNCTION_TYPE>
	void Handle<TYPE>::Call(FUNCTION_TYPE function) const
	{
		auto uid = _uid;
		Type::Daemon::Call([function, uid] (typename Type::Engine & engine) {
			auto base = engine.GetObject(uid);
			ASSERT (base != nullptr);

			auto& derived = static_cast<Type &>(* base);
			function(derived);
		});
	}
	
	// calls the given function with a pointer to the handle's object
	// (or nullptr if the handle does not reference a valid object)
	template <typename TYPE>
	template <typename FUNCTION_TYPE>
	void Handle<TYPE>::CallPtr(FUNCTION_TYPE function) const
	{
		auto uid = _uid;
		Type::Daemon::Call([function, uid] (typename Type::Engine & engine) {
			auto base = engine.GetObject(uid);
			auto derived = static_cast<Type *>(base);
			function(derived);
		});
	}
	
	// calls the given function with a reference to the handle's object
	// and stores result in given future
	template <typename TYPE>
	template <typename VALUE_TYPE, typename FUNCTION_TYPE>
	void Handle<TYPE>::Call(Future<VALUE_TYPE> & future, FUNCTION_TYPE function) const
	{
		ASSERT(future.IsPendind());
		
		auto uid = _uid;
		Type::Daemon::Call([& future, function, uid] (typename Type::Engine & engine) {
			ASSERT(future.IsPendind());
			
			auto base = engine.GetObject(uid);
			if (base == nullptr) {
				future.OnFailure();
				return;
			}
			
			auto & derived = static_cast<Type &>(* base);
			future.OnSuccess(function(derived));
		});
	}
	
	////////////////////////////////////////////////////////////////////////////////
	// Handle helpers
	
	template <typename TYPE>
	bool operator==(Handle<TYPE> const & lhs, Handle<TYPE> const & rhs)
	{
		return lhs.GetUid() == rhs.GetUid();
	}
	
	template <typename TYPE>
	bool operator!=(Handle<TYPE> const & lhs, Handle<TYPE> const & rhs)
	{
		return lhs.GetUid() != rhs.GetUid();
	}
	
	template <typename TYPE>
	bool operator<(Handle<TYPE> const & lhs, Handle<TYPE> const & rhs)
	{
		return lhs.GetUid() < rhs.GetUid();
	}
}
