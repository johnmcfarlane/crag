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
// E.g. "ipc::Handle<MyClass>" becomes "MyClassHandle".
#define DECLARE_CLASS_HANDLE(CLASS) \
	class CLASS; \
	typedef ipc::Handle<CLASS> CLASS##Handle

namespace ipc
{
	template <typename TYPE> class Handle;
}

namespace std
{
	template <typename TYPE>
	ostream & operator << (ostream & out, ::ipc::Handle<TYPE> const & handle);
}

namespace ipc
{
	// forward-declaration
	template <typename RESULT_TYPE>
	class Future;
	
	// A handle to an object on a different thread
	// which is of (or derived from) type, TYPE.
	template <typename TYPE>
	class Handle
	{
		friend struct std::hash<Handle>;

		////////////////////////////////////////////////////////////////////////////////
		// types
		
		using ObjectType = TYPE;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Handle(Uid uid)
		: _uid(uid)
		{
		}
		
	public:
		Handle() = default;
		
		friend std::ostream & operator << (std::ostream & out, Handle const & handle)
		{
			return out << handle._uid;
		}

		friend bool operator == (Handle const & lhs, Handle const & rhs)
		{
			return lhs._uid == rhs._uid;
		}
		
		friend bool operator != (Handle const & lhs, Handle const & rhs)
		{
			return lhs._uid != rhs._uid;
		}
		
		friend bool operator < (Handle const & lhs, Handle const & rhs)
		{
			return lhs._uid < rhs._uid;
		}
		
		// cast to base types
		template <typename BASE_TYPE> 
		operator Handle<BASE_TYPE> () const;

		// true iff handle points to an object
		bool IsInitialized() const
		{
			return _uid.IsInitialized();
		}
		
		static Handle CreateFromUid(Uid const & uid)
		{
			return Handle(uid);
		}
		
		// creates an object; passes parameters to c'tor;
		// returns handle to object
		template <typename ... PARAMETERS>
		static Handle Create(PARAMETERS && ... parameters);

		template <typename ... PARAMETERS>
		void CreateObject(PARAMETERS ... parameters);
		
		// Tells simulation to release the object.
		void Release();
		
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
