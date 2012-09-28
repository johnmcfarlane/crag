//
//  Uid.h
//  crag
//
//  Created by John McFarlane on 10/28/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace smp
{
	// forward-declarations
	class Uid;	
	std::ostream & operator << (std::ostream & out, Uid const & uid);

	// Unique identifier:
	// - thread-safe generation through Uid::Create;
	// - Used to refer to objects without referring to their memory address.
	class Uid
	{
		// types
		typedef uintptr_t ValueType;
        
	public:
		// functions
		Uid() : _value(0) { }
		
#if ! defined(NDEBUG)
		ValueType GetValue() const { return _value; }
#endif
		
		operator bool () const { return _value != 0; }
		
		Uid & operator = (Uid const & rhs) { _value = rhs._value; return * this; }
		
		friend bool operator == (Uid const & lhs, Uid const & rhs) { return lhs._value == rhs._value; }
		friend bool operator != (Uid const & lhs, Uid const & rhs) { return lhs._value != rhs._value; }
		friend bool operator < (Uid const & lhs, Uid const & rhs) { return lhs._value < rhs._value; }
		
		friend std::ostream & operator << (std::ostream & out, Uid const & uid);
		
		// create a new unique object
		static Uid Create();
		
	private:
		// variables
		ValueType _value;
	};
}
