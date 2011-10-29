//
//  Uid.cpp
//  crag
//
//  Created by John McFarlane on 10/28/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Uid.h"

#include "atomic.h"

#include "core/intOps.h"


smp::Uid smp::Uid::Create()
{
	static volatile ValueType _counter = 1;

#if ! defined(NDEBUG)
	// check that we've still got lots of capacity
	Assert(_counter <= ValueType(-1) >> 8);
	
	// check that GetBitCount is reliable
	Assert(GetBitCount(ValueType(-1)) == sizeof(ValueType) * CHAR_BIT);
	
	// periodically report that uids are being used up
	if (GetBitCount(_counter) == 1)
	{
		std::cout << "Uid " << _counter << std::endl;
	}
#endif

	// Instantiate a Uid (with _invalid_value).
	Uid creation;
	
	// Overwrite its _value with a unique value.
	creation._value = AtomicFetchAndAdd(_counter, 1);

	// Return unique Uid.
	return creation;
}
