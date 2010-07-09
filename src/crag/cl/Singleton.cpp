/*
 *  cl/Singleton.cpp
 *  crag
 *
 *  Created by John on 7/2/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Singleton.h"

#if (USE_OPENCL)

#include <fstream>


////////////////////////////////////////////////////////////////////////////////
// Singleton members

cl::Singleton::Singleton(cl_device_type init_device_type)
: device_type(init_device_type)
, device_id(0)
, context(0)
, queue(0)
{
	cl_uint count;

	cl_uint const max_platforms = 2;
	cl_platform_id platforms[max_platforms];
	CL_CHECK (clGetPlatformIDs (max_platforms, platforms, & count));

	Assert(count < 2);	// TODO: Add extra code to choose between multiple platforms.
	if (count == 0)
	{
		std::cerr << "Could not find an OpenCL platform. Missing driver/GPU?\n";
		device_type = CL_DEVICE_TYPE_DEFAULT;	// means don't use CL at all
		return;
	}
	
	// Used to get error back from following functions.
	cl_int err;

	// Get an ID for the device
	err = clGetDeviceIDs(platforms[0], device_type, 1, &device_id, NULL);
	switch (err)
	{
	case CL_SUCCESS:
		break;

	case CL_DEVICE_NOT_FOUND:
		std::cerr << "OpenCL error: CL_DEVICE_NOT_FOUND. Maybe try a different value for config variable, 'device_type'?\n";
		std::cerr << "\tdevice_type=" << device_type << '\n';
		std::cerr << "\tValid values are: \n";
		std::cerr << "\t\t" << CL_DEVICE_TYPE_DEFAULT << " (default)\n";
		std::cerr << "\t\t" << CL_DEVICE_TYPE_CPU << " (cpu)\n";
		std::cerr << "\t\t" << CL_DEVICE_TYPE_GPU << " (gpu)\n";
		std::cerr << "\t\t" << CL_DEVICE_TYPE_ALL << " (don't use OpenCL at all)\n";
		break;

	default:
		CL_CHECK(err);
	}

	if (device_type == CL_DEVICE_TYPE_DEFAULT)
	{
		CL_CHECK (clGetDeviceInfo(device_id, CL_DEVICE_TYPE, sizeof(cl_device_type), & device_type, NULL));
		std::cout << "device_type=" << device_type << '\n';
	}

	// Create a context
	context = clCreateContext (0, 1, &device_id, NULL, NULL, &err);
	CL_CHECK (err);
	
	// Create a command queue
	queue = clCreateCommandQueue(context, device_id, 0, &err);
	CL_CHECK (err);
}

cl::Singleton::~Singleton()
{
	CL_CHECK (clReleaseCommandQueue(queue));
	CL_CHECK (clReleaseContext(context));
}

cl_device_type cl::Singleton::GetDeviceType() const
{
	return device_type;
}

#endif
