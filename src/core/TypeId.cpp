//
//  core/TypeId.cpp
//  crag
//
//  Created by John McFarlane on 2014-01-18.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "TypeId.h"

using namespace crag::core;

////////////////////////////////////////////////////////////////////////////////
// crag::core::TypeId member definitions

TypeId::TypeId(int value) 
: _value(value) 
{ 
}

TypeId::TypeId() 
: _value(0) 
{ 
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(TypeId, self)
	CRAG_VERIFY_OP(self._value, >, 0);
CRAG_VERIFY_INVARIANTS_DEFINE_END

////////////////////////////////////////////////////////////////////////////////
// crag::core::TypeId friend definitions

bool ::crag::core::operator<(TypeId const & lhs, TypeId const & rhs)
{
	return lhs._value < rhs._value;
}

bool ::crag::core::operator==(TypeId const & lhs, TypeId const & rhs)
{
	return lhs._value == rhs._value;
}

bool ::crag::core::operator!=(TypeId const & lhs, TypeId const & rhs)
{
	return lhs._value != rhs._value;
}

std::ostream & crag::core::operator << (std::ostream & lhs, TypeId const & rhs)
{
	return lhs << rhs._value;
}

std::istream & crag::core::operator >> (std::istream & lhs, TypeId & rhs)
{
	return lhs >> rhs._value;
}

////////////////////////////////////////////////////////////////////////////////
// crag::core::GetUniqueId() definition

namespace crag
{
	namespace core
	{
		// not thread-safe
		int GetUniqueId()
		{
			static int id_counter = 0;
			++ id_counter;
			return id_counter;
		}
	}
}
