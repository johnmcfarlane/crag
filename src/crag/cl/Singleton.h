/*
 *  cl/Singleton.h
 *  crag
 *
 *  Created by John on 7/2/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "defs.h"

#if (USE_OPENCL)

#include "core/Singleton.h"


namespace cl 
{
	// forward-declares
	
	
	class Singleton : public core::Singleton<Singleton>
	{
		friend class Kernel;
		friend class GpuKernel;
		friend class CpuKernel;
		
	public:
		Singleton(cl_device_type device_type);
		~Singleton();
		
		cl_device_type GetDeviceType() const;
		
	private:
		
		cl_device_type device_type;
		cl_device_id device_id;
		cl_context context;
		cl_command_queue queue;
	};
	
}

#endif
