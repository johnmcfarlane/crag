/*
 *  defs.cpp
 *  crag
 *
 *  Created by John on 7/10/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "defs.h"

#include "core/ConfigEntry.h"


// cl_device_type text<->value converters for the config systems.
template<> int Config<cl_device_type>::ValueToString(char * string) const
{
	//return sprintf(string, "%I64d", var) > 0;
	return sprintf(string, "%llu", var) > 0;
}

template<> int Config<cl_device_type>::StringToValue(char const * string)
{
	//return sscanf(string, "%I64d", & var) == 1;
	return sscanf(string, "%llu", & var) == 1;
}

