//
//  Handle.h
//  crag
//
//  Created by John McFarlane on 2/12/12.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Future.h"
#include "Uid.h"


// Forward-declares class, CLASS, and creates shorthand version of its handle,
// E.g. "smp::Handle<MyClass>" becomes "MyClassHandle".
#define DECLARE_CLASS_HANDLE(CLASS) \
	class CLASS; \
	typedef smp::Handle<CLASS> CLASS##Handle

namespace smp
{
	// forward-declaration
	template <typename RESULT_TYPE>
	class Future;

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
		
#if defined(__GNUC__)
		void Create();

		template <typename PARAMETER1>
		void Create(PARAMETER1 parameter1);

		template <typename PARAMETER1, typename PARAMETER2>
		void Create(PARAMETER1 parameter1, PARAMETER2 parameter2);

		template <typename PARAMETER1, typename PARAMETER2, typename PARAMETER3>
		void Create(PARAMETER1 parameter1, PARAMETER2 parameter2, PARAMETER3 parameter3);
#else
		template <typename ... PARAMETERS>
		void Create(PARAMETERS ... parameters);
#endif
		
		// Tells simulation to destroy the object.
		void Destroy();
		
		////////////////////////////////////////////////////////////////////////////////
		// Call - generates a deferred function call to the thread-safe object
		
		// calls the given function with a reference to the handle's object
		template <typename FUNCTION_TYPE>
		void Call(FUNCTION_TYPE function) const;
		
		// calls the given function with a pointer to the handle's object
		// (or nullptr if the handle does not reference a valid object)
		template <typename FUNCTION_TYPE>
		void CallPtr(FUNCTION_TYPE function) const;
		
		// calls the given function with a reference to the handle's object
		// and stores result in given future
		template <typename VALUE_TYPE, typename FUNCTION_TYPE>
		void Call(Future<VALUE_TYPE> & result, FUNCTION_TYPE function) const;
		
	private:
		// Call helpers
		template <typename FUNCTION_TYPE>
		class CallCommand;
		
		template <typename VALUE_TYPE, typename FUNCTION_TYPE>
		class PollCommand;

		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		Uid _uid;
	};
}
