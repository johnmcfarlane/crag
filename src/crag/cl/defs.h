/*
 *  defs.h
 *  crag
 *
 *  Created by John on 7/7/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


//#if defined(WIN32) || (defined(__APPLE__) && ! defined(__ppc__))
//#define USE_OPENCL
//#endif


#if defined(USE_OPENCL)

#error Not currently using OpenCL.

#if defined(__APPLE__)
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif


#if defined(NDEBUG)
#define CL_CHECK(EXPRESSION) (EXPRESSION == CL_SUCCESS)
#else

#include "core/debug.h"

#define CL_CHECK(EXPRESSION) CheckResult(EXPRESSION, #EXPRESSION, __FILE__, __LINE__)

namespace cl
{
	inline bool CheckResult(cl_int error_code, char const * expression, char const * file, int line)
	{
		if (error_code == CL_SUCCESS)
		{
			return true;
		}
		else 
		{
			std::cerr << file << ":" << line << ": cl error: \"" << expression << "\" = " << error_code << '\n';
			DEBUG_BREAK();
			return false;
		}
	}
}

#endif

#endif
