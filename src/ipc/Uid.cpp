//
//  Uid.cpp
//  crag
//
//  Created by John McFarlane on 10/28/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Uid.h"

using namespace ipc;

////////////////////////////////////////////////////////////////////////////////
// ipc::Uid friends

std::ostream & ipc::operator << (std::ostream & out, Uid const & uid)
{
	return out << uid._value;
}

std::size_t std::hash<ipc::Uid>::operator() (ipc::Uid uid) const
{
	ASSERT(uid.IsInitialized());
	return uid._value;
}


////////////////////////////////////////////////////////////////////////////////
// ipc::Uid member definitions

Uid Uid::Create()
{
	static std::atomic<ValueType> _counter(2u);
	
#if defined(CRAG_DEBUG)
	// check that we've still got lots of capacity
	ASSERT(_counter <= ValueType(-1) >> 8);
	
	// check that GetBitCount is reliable
	ASSERT(GetBitCount(ValueType(-1)) == std::numeric_limits<ValueType>::digits);
	
	// periodically report that uids are being used up
	if (_counter > 1000000 && GetBitCount<ValueType>(_counter) == 1)
	{
		DEBUG_MESSAGE("Uid %lf", static_cast<double>(_counter));
	}
#endif
	
	// Instantiate a Uid (with _invalid_value).
	Uid creation;
	
	// Overwrite its _value with a unique value.
	creation._value = std::atomic_fetch_add(& _counter, ValueType(1));
	
	// Return unique Uid.
	return creation;
}
