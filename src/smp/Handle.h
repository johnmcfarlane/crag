//
//  Handle.h
//  crag
//
//  Created by John McFarlane on 2/12/12.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "smp/Message.h"
#include "smp/Uid.h"


// Forward-declares class, CLASS, and creates shorthand version of its handle.
// E.g. "smp::Handle<ns::MyObject>" becomes "ns::MyObjectHandle".
#define DECLARE_CLASS_HANDLE(NAMESPACE, CLASS) \
	namespace NAMESPACE { \
		class CLASS; \
		typedef smp::Handle<CLASS> CLASS##Handle; \
	}


namespace smp
{
	// A handle to an object on a different thread 
	// which is of (or derived from) type, TYPE.
	template <typename TYPE>
	class Handle
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef TYPE Type;

	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Handle()
		{
		}
		
		// TODO: Probably a bad idea...
		Handle(Uid uid)
		: _uid(uid)
		{
		}
		
		Handle(Type & object)
		: _uid(object.GetUid())
		{
		}
		
		~Handle()
		{
		}
		
		template <typename BASE_TYPE>
		operator Handle<BASE_TYPE> & ()
		{
			ASSERT((BASE_TYPE *)((Type *)nullptr) == nullptr);
			return reinterpret_cast<Handle <BASE_TYPE> &>(* this);
		}
		
		template <typename BASE_TYPE>
		operator Handle<BASE_TYPE> const & () const
		{
			ASSERT((BASE_TYPE *)((Type *)nullptr) == nullptr);
			return reinterpret_cast<Handle <BASE_TYPE> const &>(* this);
		}
		
		operator bool () const
		{
			return _uid;
		}

		// Returns the UID of the entity being handled.
		Uid GetUid() const
		{
			return _uid;
		}
		
		// Sets the UID of the entity being handled.
		// TODO: ... but then so is this.
		void SetUid(Uid uid)
		{
			_uid = uid;
		}
		
		template <typename INIT_DATA>
		void Create(INIT_DATA const & init_data)
		{
			typedef typename Type::Daemon Daemon;
			typedef typename Daemon::Class DaemonClass;

			Destroy();
			Uid uid = Uid::Create();
			SetUid(uid);
			Daemon::Call(& DaemonClass::template OnCreateObject<Type, INIT_DATA>, uid, init_data);
		}

		// Tells simulation to destroy the object.
		void Destroy()
		{
			typedef typename Type::Daemon Daemon;
			typedef typename Daemon::Class DaemonClass;

			// If not already destroyed,
			if (_uid)
			{
				// set message.
				Daemon::Call(& DaemonClass::OnRemoveObject, _uid);
				_uid = Uid();
			}
		}
		
		
		////////////////////////////////////////////////////////////////////////////////
		// Call - generates a deferred function call to the thread-safe object
		
		// general purpose functor
		template <typename FUNCTOR>
		void Call(FUNCTOR const & functor) const
		{
			CallMessageFunctor<FUNCTOR> message(_uid, functor);
			Type::Daemon::SendMessage(message);
		}
		
		// 0-parameter version
		void Call (void (Type::* function)()) const
		{
			CallMessage0 message(function);
			Call(message);
		}
		
		// 1-parameter version
		template <typename PARAMETER1>
		void Call (void (Type::* function)(PARAMETER1 const &), PARAMETER1 const & parameter1) const
		{
			CallMessage1<PARAMETER1> message(function, parameter1);
			Call(message);
		}
		
		// 2-parameter version
		template <typename PARAMETER1, typename PARAMETER2>
		void Call (void (Type::* function)(PARAMETER1 const &, PARAMETER2 const &), PARAMETER1 const & parameter1, PARAMETER2 const & parameter2) const
		{
			CallMessage2<PARAMETER1, PARAMETER2> message(function, parameter1, parameter2);
			Call(message);
		}
		
		// 3-parameter version
		template <typename PARAMETER1, typename PARAMETER2, typename PARAMETER3>
		void Call (PARAMETER1 const & parameter1, PARAMETER2 const & parameter2, PARAMETER3 const & parameter3, void (Type::* function)(PARAMETER1 const &, PARAMETER2 const &, PARAMETER3 const &)) const
		{
			CallMessage3<PARAMETER1, PARAMETER2, PARAMETER3> message(function, parameter1, parameter2, parameter3);
			Call(message);
		}
		
	private:
		template <typename FUNCTOR>
		class CallMessageFunctor : public smp::Message<typename Type::DaemonClass>
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
			virtual void operator () (typename Type::DaemonClass & daemon_class) const override
			{
				Type * derived = static_cast<Type *>(daemon_class.GetObject(_uid));
				_functor(derived);
			}
			Uid _uid;
			Functor _functor;
		};
		
		// 0-parameter Call helper
		class CallMessage0
		{
		public:
			typedef void (Type::* FunctionType)();
			CallMessage0(FunctionType function)
			: _function(function)
			{
			}
			
			void operator () (Type * derived) const
			{
				if (derived != nullptr)
				{
					(derived->*_function)();
				}
			}
		private:
			FunctionType _function;
		};
		
		// 1-parameter Call helper
		template <typename PARAMETER1>
		class CallMessage1
		{
		public:
			typedef void (Type::* FunctionType)(PARAMETER1 const &);
			CallMessage1(FunctionType function, PARAMETER1 const & __parameter1) 
			: _function(function)
			, _parameter1(__parameter1) 
			{ 
			}
			
			void operator () (Type * derived) const 
			{
				if (derived != nullptr)
				{
					(derived->*_function)(_parameter1);
				}
			}
		private:
			FunctionType _function;
			PARAMETER1 _parameter1;
		};
		
		// 2-parameter Call helper
		template <typename PARAMETER1, typename PARAMETER2>
		class CallMessage2
		{
		public:
			typedef void (Type::* FunctionType)(PARAMETER1 const &, PARAMETER2 const &);
			CallMessage2(FunctionType function, PARAMETER1 const & __parameter1, PARAMETER2 const & __parameter2) 
			: _function(function)
			, _parameter1(__parameter1)
			, _parameter2(__parameter2) 
			{ 
			}
			
			void operator () (Type * derived) const 
			{
				if (derived != nullptr)
				{
					(derived->*_function)(_parameter1, _parameter2);
				}
			}
		private:
			FunctionType _function;
			PARAMETER1 _parameter1;
			PARAMETER2 _parameter2;
		};
		
		// 3-parameter Call helper
		template <typename PARAMETER1, typename PARAMETER2, typename PARAMETER3>
		class CallMessage3
		{
		public:
			typedef void (Type::* FunctionType)(PARAMETER1 const &, PARAMETER2 const &, PARAMETER3 const &);
			CallMessage3(FunctionType function, PARAMETER1 const & __parameter1, PARAMETER2 const & __parameter2, PARAMETER3 const & __parameter3) 
			: _function(function)
			, _parameter1(__parameter1)
			, _parameter2(__parameter2)
			, _parameter3(__parameter3) 
			{ 
			}

			void operator () (Type * derived) const 
			{
				if (derived != nullptr)
				{
					(derived->*_function)(_parameter1, _parameter2, _parameter3);
				}
			}
		private:
			FunctionType _function;
			PARAMETER1 _parameter1;
			PARAMETER2 _parameter2;
			PARAMETER3 _parameter3;
		};


		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Uid _uid;
	};
	
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
