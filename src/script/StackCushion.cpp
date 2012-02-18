//
//  StackCushion.cpp
//  crag
//
//  Created by John McFarlane on 2012-02-10.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "StackCushion.h"


using namespace script;


////////////////////////////////////////////////////////////////////////////////
// script::StackCushion member definitions

StackCushion::StackCushion()
{
	for (char * i = SafeMarginBegin(), * end = SafeMarginEnd(); i != end; ++ i)
	{
		* i = _marker_value;
	}
	
	Assert(IsSafe());
}

bool StackCushion::IsSafe() const
{
	for (char const * i = SafeMarginBegin(), * end = SafeMarginEnd(); i != end; ++ i)
	{
		if (* i != _marker_value)
		{
			return false;
		}
	}
	
	return true;
}

// NOTE: This SafeMarginBegin/End likely only works for architectures with negative stack growth.
// For positive-growth architectures, try [_cushion_buffer + _cushion_size - _safe_margin_size, _cushion_buffer + _cushion_size).
char * StackCushion::SafeMarginBegin()
{
	return _cushion_buffer;
}

char const * StackCushion::SafeMarginBegin() const
{
	return _cushion_buffer;
}

char * StackCushion::SafeMarginEnd()
{
	return _cushion_buffer + _safe_margin_size;
}

char const * StackCushion::SafeMarginEnd() const
{
	return _cushion_buffer + _safe_margin_size;
}

////////////////////////////////////////////////////////////////////////////////
// constants

// A multiplier which accounts for the fact that more stack is used in debug builds.
#if defined(NDEBUG)
static const size_t _debug_factor = 1;
#else
static const size_t _debug_factor = 2;
#endif

// The size of the cushion in bytes.
static const size_t _cushion_size = 128 * sizeof(void *) * _debug_factor;

// The size of the area of the cushion intended to remain untouched.
static const size_t _safe_margin_size = _cushion_size >> 2;

// The value written into the safe margin.
static const char _marker_value = 0x9d;
