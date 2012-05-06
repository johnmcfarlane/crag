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
	void Handle<TYPE>::Create()
	{
		typedef typename Type::Daemon Daemon;
		typedef typename Type::Engine Engine;
		
		Destroy();
		Uid uid = Uid::Create();
		SetUid(uid);
		Daemon::Call(& Engine::template OnCreateObject<Type>, uid);
	}

	template <typename TYPE>
	template <typename INIT_DATA>
	void Handle<TYPE>::Create(INIT_DATA const & init_data)
	{
		typedef typename Type::Daemon Daemon;
		typedef typename Type::Engine Engine;
		
		Destroy();
		Uid uid = Uid::Create();
		SetUid(uid);
		Daemon::Call(& Engine::template OnCreateObject<Type, INIT_DATA>, uid, init_data);
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
			Daemon::Call(& Engine::OnRemoveObject, _uid);
			_uid = Uid();
		}
	}
	
	// general purpose functor
	template <typename TYPE>
	template <typename FUNCTOR>
	void Handle<TYPE>::Call(FUNCTOR const & functor) const
	{
		CallMessageFunctor<FUNCTOR> message(_uid, functor);
		Type::Daemon::SendMessage(message);
	}
	
	// calls the given function with the given parameters
	template <typename TYPE>
	template <typename... PARAMETERS>
	void Handle<TYPE>::Call(void (Type::* function)(PARAMETERS const & ...), PARAMETERS const &... parameters) const
	{
		CallCommand<PARAMETERS ...> command(function, parameters ...);
		Call(command);
	}
	
	// calls a function which returns a value
	template <typename TYPE>
	template <typename VALUE_TYPE, typename FUNCTION_TYPE, typename... PARAMETERS>
	void Handle<TYPE>::Poll(VALUE_TYPE & result, PollStatus & status, FUNCTION_TYPE function, PARAMETERS const &... parameters) const
	{
		PollCommand<VALUE_TYPE, FUNCTION_TYPE, PARAMETERS ...> command(result, status, function, parameters ...);
		Call(command);
	}
	
	template <typename TYPE>
	template <typename FUNCTOR>
	class Handle<TYPE>::CallMessageFunctor : public smp::Message<typename Type::Engine>
	{
	public:
		typedef FUNCTOR Functor;
		CallMessageFunctor(Uid const & uid, Functor const & functor)
		: _uid(uid)
		, _functor(functor)
		{
			ASSERT(_uid);
		}
	private:
		virtual void operator () (typename Type::Engine & daemon_class) const override
		{
			Type * derived = static_cast<Type *>(daemon_class.GetObject(_uid));
			_functor(derived);
		}
		Uid _uid;
		Functor _functor;
	};
	
	// 1-parameter Call helper
	template <typename TYPE>
	template <typename ... PARAMETERS>
	class Handle<TYPE>::CallCommand
	{
		// types
		typedef void (Type::* FunctionType)(PARAMETERS const & ...);
	public:
		// functions
		CallCommand(FunctionType function, PARAMETERS const & ... parameters) 
		: _function(function)
		, _parameters(parameters...)
		{ 
		}
		void operator () (Type * derived) const 
		{
			if (derived != nullptr)
			{
				core::call(* derived, _function, _parameters);
			}
		}
	private:
		// variables
		FunctionType _function;
		std::tr1::tuple<PARAMETERS...> _parameters;
	};
	
	template <typename TYPE>
	template <typename VALUE_TYPE, typename FUNCTION_TYPE, typename ... PARAMETERS>
	class Handle<TYPE>::PollCommand
	{
	public:
		// functions
		PollCommand(VALUE_TYPE & result, PollStatus & status, FUNCTION_TYPE function, PARAMETERS const & ... parameters) 
		: _result(result)
		, _status(status)
		, _function(function)
		, _parameters(parameters...)
		{ 
		}
		void operator () (Type * derived) const
		{
			ASSERT(_status == pending);
			if (derived == nullptr)
			{
				_status = failed;
			}
			else
			{
				_result = core::call(* derived, _function, _parameters);
				AtomicCompilerBarrier();
				_status = complete;
			}
		}
	private:
		// variables
		VALUE_TYPE & _result;
		PollStatus & _status;
		FUNCTION_TYPE _function;
		std::tr1::tuple<PARAMETERS...> _parameters;
	};

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
