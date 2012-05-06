//
//  Handle.h
//  crag
//
//  Created by John McFarlane on 2/12/12.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "atomic.h"
#include "Message.h"
#include "PollStatus.h"
#include "Uid.h"


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
		
		Handle();
		Handle(Uid uid);
		Handle(Type & object);
		
		// static cast to base types
		template <typename BASE_TYPE> 
		operator Handle<BASE_TYPE> & ();
		
		template <typename BASE_TYPE> 
		operator Handle<BASE_TYPE> const & () const;

		// true iff handle points to an object
		operator bool () const;
		
		// Returns the UID of the entity being handled.
		Uid GetUid() const;
		
		// Sets the UID of the entity being handled.
		void SetUid(Uid uid);
		
		void Create();
		
		template <typename INIT_DATA>
		void Create(INIT_DATA const & init_data);
		
		// Tells simulation to destroy the object.
		void Destroy();
		
		////////////////////////////////////////////////////////////////////////////////
		// Call/Poll - generates a deferred function call to the thread-safe object
		
		// general purpose functor
		template <typename FUNCTOR> 
		void Call(FUNCTOR const & functor) const;
		
		// calls the given function with the given parameters
		template <typename... PARAMETERS>
		void Call(void (Type::* function)(PARAMETERS const & ...), PARAMETERS const &... parameters) const;
		
		// calls a function which returns a value
		template <typename VALUE_TYPE, typename FUNCTION_TYPE, typename... PARAMETERS>
		void Poll(VALUE_TYPE & result, PollStatus & status, FUNCTION_TYPE function, PARAMETERS const &... parameters) const;		
	private:
		template <typename FUNCTOR>
		class CallMessageFunctor;
		
		// 1-parameter Call helper
		template <typename ... PARAMETERS>
		class CallCommand;
		
		template <typename VALUE_TYPE, typename FUNCTION_TYPE, typename ... PARAMETERS>
		class PollCommand;

		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Uid _uid;
	};
}
