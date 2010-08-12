/*
 *  CpuKernel.cpp
 *  crag
 *
 *  Created by John on 7/6/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "CpuKernel.h"

#if (USE_OPENCL)

#include "Singleton.h"


////////////////////////////////////////////////////////////////////////////////
// CpuKernel members

cl::CpuKernel::CpuKernel(int init_max_elements, size_t init_element_size, void * init_buffer)
: Kernel()
, max_elements(init_max_elements)
, element_size(init_element_size)
, buffer(init_buffer)
{
	Singleton & singleton = Singleton::Get();
	
	// Create the input and output arrays in device memory for our calculation
	buffer_object = clCreateBuffer(singleton.context, CL_MEM_USE_HOST_PTR, init_element_size * init_max_elements, init_buffer, NULL);
	if (! buffer_object)
	{
		std::cerr << "error: clCreateBuffer failed to create input\n";
		return;// false;
	}
}

void cl::CpuKernel::Process(int num_elements)
{
	Assert(num_elements < max_elements);
	
	Singleton & singleton = Singleton::Get();
	
	// Write our data set into the input array in device memory.
	int err = clEnqueueWriteBuffer (singleton.queue, buffer_object, CL_TRUE, 0, element_size * num_elements, buffer, 0, NULL, NULL);
	CL_CHECK (err);
	
	// Set the arguments to our compute kernel
	CL_CHECK (clSetKernelArg (kernel, 0, sizeof(cl_mem), & buffer_object));
	
	CL_CHECK (clSetKernelArg (kernel, 1, sizeof(unsigned int), & num_elements));
	
	SetAdditionalArgs();
	
	Kernel::Process(num_elements);
	
	// Read the results from the device
	CL_CHECK (clEnqueueReadBuffer (singleton.queue, buffer_object, CL_TRUE, 0, element_size * num_elements, buffer, 0, NULL, NULL));
}

void cl::CpuKernel::SetAdditionalArgs()
{
}

#endif
