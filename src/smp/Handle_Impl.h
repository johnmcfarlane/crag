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
	
	template <typename TYPE>
	template <typename ... PARAMETERS>
	void Handle<TYPE>::Create(PARAMETERS const & ... parameters)
	{
		typedef typename Type::Daemon Daemon;
		typedef typename Type::Engine Engine;
		
		Destroy();
		Uid uid = Uid::Create();
		SetUid(uid);
		Daemon::Call([uid, parameters ...] (Engine & engine) {
			engine.template CreateObject<Type, PARAMETERS ...>(uid, parameters ...);
		});
	}
	
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
	
	// calls the given function with the given parameters
	template <typename TYPE>
	template <typename FUNCTION_TYPE>
	void Handle<TYPE>::Call(FUNCTION_TYPE function) const
	{
		auto uid = _uid;
		Type::Daemon::Call([function, uid] (typename Type::Engine & engine) {
			auto * derived = static_cast<Type *>(engine.GetObject(uid));
			if (derived != nullptr) {
				function(* derived);
			}
		});
	}
	
	// calls a function which returns a value
	template <typename TYPE>
	template <typename VALUE_TYPE, typename FUNCTION_TYPE>
	void Handle<TYPE>::Poll(VALUE_TYPE & result, PollStatus & status, FUNCTION_TYPE function) const
	{
		PollCommand<VALUE_TYPE, FUNCTION_TYPE> command(result, status, function);
		Call(command);
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
