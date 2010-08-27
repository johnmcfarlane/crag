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


#if defined(USE_OPENCL)

// cl_device_type text<->value converters for the config systems.
template<> int Config<cl_device_type>::ValueToString(char * string, cl_device_type const & value) 
{
	//return sprintf(string, "%I64d", var) > 0;
	return sprintf(string, "%llu", value) > 0;
}

template<> int Config<cl_device_type>::StringToValue(cl_device_type & value, char const * string)
{
	//return sscanf(string, "%I64d", & var) == 1;
	return sscanf(string, "%llu", & value) == 1;
}
#endif
