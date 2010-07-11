/*
 *  cl/Kernel.cpp
 *  crag
 *
 *  Created by John on 7/4/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Kernel.h"

#if (USE_OPENCL)

#include "Singleton.h"

#include <fstream>


////////////////////////////////////////////////////////////////////////////////
// Kernel members

cl::Kernel::Kernel(char const * kernel_source)
{
	int err;
	Singleton & singleton = Singleton::Get();
	
	// Create the compute program from the source buffer
	program = clCreateProgramWithSource(singleton.context, 1, (const char **) & kernel_source, NULL, &err);
	CL_CHECK(err);
	
	// Build the program executable
	if (! CL_CHECK(clBuildProgram(program, 0, NULL, NULL, NULL, NULL)))
	{
		size_t len;
		char buffer[2048];
		
		clGetProgramBuildInfo(program, singleton.device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
		std::cerr << buffer << '\n';
	}
	
	// Create the compute kernel in the program we wish to run
	kernel = clCreateKernel(program, "score_nodes", &err);
	CL_CHECK(err);
}

cl::Kernel::~Kernel()
{
	clReleaseProgram(program);
	clReleaseKernel(kernel);
}

void cl::Kernel::Process(int num_elements)
{
	if (num_elements == 0)
	{
		return;
	}
	
	Singleton & singleton = Singleton::Get();

	size_t global;                    // global domain size for our calculation
	size_t local;                     // local domain size for our calculation

	// Get the maximum work-group size for executing the kernel on the device
	CL_CHECK(clGetKernelWorkGroupInfo(kernel, singleton.device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(int), & local, NULL));
	
	//CL_CHECK(clGetDeviceInfo(singleton.device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(int), & local, NULL));	
	
	// Execute the kernel over the entire range of the data set
	global = num_elements;
	CL_CHECK(clEnqueueNDRangeKernel(singleton.queue, kernel, 1, NULL, & global, NULL/*& local*/, 0, NULL, NULL));

	// Wait for the command queue to get serviced before reading back results
	CL_CHECK(clFinish(singleton.queue));
}

// Load the contents of a .cl file into memory.
char * cl::Kernel::LoadClFile(char const * filename)
{
	// Open the cl file
	std::ifstream in_file(filename, std::ios::in|std::ios::binary|std::ios::ate);
	if (! in_file.is_open())
	{
		return nullptr;
	}

	// Get the file size and create a buffer big enough to load it.
	size_t file_size = static_cast<size_t>(in_file.tellg());
	char * kernel_source = new char [file_size + 1];
	
	// Load the file as a string.
	in_file.seekg (0, std::ios::beg);
	in_file.read (kernel_source, file_size);
	in_file.close();

	// Terminate the string and return.
	kernel_source[file_size] = '\0';
	return kernel_source;
}

bool cl::Kernel::SaveCFile(char const * filename, char const * kernel_source, char const * string_name)
{
	// Open the cpp file.
	std::ofstream out_file(filename, std::ios::out);
	if (! out_file.is_open())
	{
		return false;
	}

	out_file << "char const * " << string_name << " = \"";
	for (char const * c = kernel_source; (* c) != '\0'; ++ c)
	{
		if ((* c) == '\n')
		{
			out_file << "\\n\" \\\n\"";
		}
		else 
		{
			out_file << (* c);
		}
	}
	
	out_file << "\";\n";
	out_file.close();
	
	// TODO: A little less optimizm? 
	return true;
}

void cl::Kernel::SetAdditionalArg(int index, size_t size, void const * value)
{
	CL_CHECK(clSetKernelArg(kernel, index, size, value));
}

#endif
