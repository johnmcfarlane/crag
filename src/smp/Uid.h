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
	// Unique identifier:
	// - thread-safe generation through Uid::Create;
	// - Used to refer to objects without referring to their memory address.
	class Uid
	{
		// types
		typedef unsigned int ValueType;
        
	public:
		// functions
		Uid() : _value(0) { }
		Uid(Uid const & rhs) : _value(rhs._value) { }
		
		bool operator == (Uid const & rhs) const { return _value == rhs._value; }
		bool operator != (Uid const & rhs) const { return _value != rhs._value; }

		friend std::ostream & operator << (std::ostream & out, Uid const & uid);

		// create a new unique object
		static Uid Create();
		
	private:
		// variables
		volatile ValueType _value;
	};
}