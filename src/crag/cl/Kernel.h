/*
 *  cl/Kernel.h
 *  crag
 *
 *  Created by John on 7/4/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "defs.h"

#if (USE_OPENCL)


namespace cl
{
	
	class Kernel
	{
	public:
		Kernel();
		virtual ~Kernel();
		
		bool Compile(char const * kernel_source);
		void Process(int num_elements);
		
		static char * LoadClFile(char const * filename);	// don't forget to delete the returned string!
		static bool SaveCFile(char const * filename, char const * kernel_source, char const * string_name);
		
	protected:
		void SetAdditionalArg(int index, size_t size, void const * value);
		
	protected:
		cl_program program;
		cl_kernel kernel;
	};
	
}

#endif
