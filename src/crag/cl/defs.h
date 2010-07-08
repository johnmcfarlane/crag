/*
 *  defs.h
 *  crag
 *
 *  Created by John on 7/7/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once


#if defined(__ppc__)
#define USE_OPENCL 0
#elif defined(__i386__) || defined(__x86_64__)
#define USE_OPENCL 1
#endif


#if USE_OPENCL

#include <OpenCL/opencl.h>

#if defined(NDEBUG)
#define CL_CHECK(EXPRESSION) EXPRESSION
#else

//#include "core/debug.h"

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
			assert(false);
			return false;
		}
	}
}

#endif

#endif
