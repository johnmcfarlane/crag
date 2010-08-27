/*
 *  GpuKernel.cpp
 *  crag
 *
 *  Created by John on 7/6/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "GpuKernel.h"

#if defined(USE_OPENCL)

#include "Singleton.h"


////////////////////////////////////////////////////////////////////////////////
// GpuKernel members

cl::GpuKernel::GpuKernel(int init_max_elements, size_t init_input_size, size_t init_output_size)
: Kernel()
, max_elements(init_max_elements)
, input_size(init_input_size)
, output_size(init_output_size)
{
	Singleton & singleton = Singleton::Get();
	
	// Create the input and output arrays in device memory for our calculation
	input = clCreateBuffer(singleton.context,  CL_MEM_READ_ONLY, input_size * max_elements, NULL, NULL);
	if (! input)
	{
		std::cerr << "error: clCreateBuffer failed to create input\n";
		return;// false;
	}
	output = clCreateBuffer(singleton.context, CL_MEM_WRITE_ONLY, output_size * max_elements, NULL, NULL);
	if (! output)
	{
		std::cerr << "error: clCreateBuffer failed to create output\n";
		return;// false;
	}
}

cl::GpuKernel::~GpuKernel()
{
	clReleaseMemObject(input);
	clReleaseMemObject(output);
}

void cl::GpuKernel::Process(void const * input_data, void * output_data, int num_elements)
{
	if (num_elements == 0)
	{
		return;
	}
	
	Assert(num_elements < max_elements);
	
	Singleton & singleton = Singleton::Get();
	
	// Write our data set into the input array in device memory.
	CL_CHECK (clEnqueueWriteBuffer(singleton.queue, input, CL_TRUE, 0, input_size * num_elements, input_data, 0, NULL, NULL));
	
	// Set the arguments to our compute kernel
	CL_CHECK (clSetKernelArg(kernel, 0, sizeof(cl_mem), & input));
	
	CL_CHECK (clSetKernelArg(kernel, 1, sizeof(cl_mem), & output));
	
	CL_CHECK (clSetKernelArg(kernel, 2, sizeof(unsigned int), & num_elements));
	
	SetAdditionalArgs();
	Kernel::Process(num_elements);
	
	// Read the results from the device
	CL_CHECK (clEnqueueReadBuffer (singleton.queue, output, CL_TRUE, 0, output_size * num_elements, output_data, 0, NULL, NULL));
}

void cl::GpuKernel::SetAdditionalArgs()
{
}

#endif
