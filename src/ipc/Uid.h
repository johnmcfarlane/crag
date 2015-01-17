//
//  Uid.h
//  crag
//
//  Created by John McFarlane on 10/28/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

// forward-declarations
namespace ipc
{
	class Uid;	
}

namespace std
{
	template <>
	struct hash <ipc::Uid>;
}

namespace ipc
{
	std::ostream & operator << (std::ostream & out, Uid const & uid);

	// Unique identifier:
	// - thread-safe generation through Uid::Create;
	// - Used to refer to objects without referring to their memory address.
	class Uid
	{
		// types
		typedef std::size_t ValueType;
		
	public:
		// functions
		bool IsInitialized() const { return _value != 0; }
		
		friend bool operator == (Uid const & lhs, Uid const & rhs) { return lhs._value == rhs._value; }
		friend bool operator != (Uid const & lhs, Uid const & rhs) { return lhs._value != rhs._value; }
		friend bool operator < (Uid const & lhs, Uid const & rhs) { return lhs._value < rhs._value; }
		
		friend std::ostream & operator << (std::ostream & out, Uid const & uid);
		friend struct std::hash<Uid>;
		
		// create a new unique object
		static Uid Create();
		
#if defined(CRAG_DEBUG)
		ValueType GetValue() const
		{
			return _value;
		}
#endif
		
	private:
		// variables
		ValueType _value = 0;
	};
}

namespace std
{
	template <>
	struct hash <ipc::Uid>
	{
		size_t operator() (ipc::Uid uid) const;
	};
}
