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

#include "core/ConfigEntry.h"

#include <fstream>


namespace
{
//																				  25000 100000
//	CONFIG_DEFINE (opencl_device_type, cl_device_type, CL_DEVICE_TYPE_DEFAULT);	// .091   .4
	CONFIG_DEFINE (opencl_device_type, cl_device_type, CL_DEVICE_TYPE_CPU);		// .06	  .31
//	CONFIG_DEFINE (opencl_device_type, cl_device_type, CL_DEVICE_TYPE_GPU);		// .1	  .47
//	CONFIG_DEFINE (opencl_device_type, cl_device_type, CL_DEVICE_TYPE_ALL);		// means don't use OpenCL
}


////////////////////////////////////////////////////////////////////////////////
// Singleton members

cl::Singleton::Singleton()
: device_type(opencl_device_type)
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
		device_type = CL_DEVICE_TYPE_ALL;	// means don't use CL at all
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
		std::cout << "OpenCL error: CL_DEVICE_NOT_FOUND. Maybe try a different value for config variable, 'device_type'?\n";
		std::cout << "\tdevice_type=" << device_type << '\n';
		std::cout << "\tValid values are: \n";
		std::cout << "\t\t" << CL_DEVICE_TYPE_DEFAULT << " (default)\n";
		std::cout << "\t\t" << CL_DEVICE_TYPE_CPU << " (cpu)\n";
		std::cout << "\t\t" << CL_DEVICE_TYPE_GPU << " (gpu)\n";
		std::cout << "\t\t" << CL_DEVICE_TYPE_ALL << " (don't use OpenCL at all)\n";
		device_type = CL_DEVICE_TYPE_ALL;	// means don't use CL at all
		return;

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

	cl_device_info param_names[] = 
	{
		CL_DEVICE_NAME,
		CL_DEVICE_PROFILE,
		CL_DEVICE_VENDOR,
		CL_DEVICE_VERSION,
		CL_DRIVER_VERSION
	};
	int const num_categories = ARRAY_SIZE(param_names);

	int const param_value_size = 1024;
	char param_value[param_value_size];
	for (int i = 0; i < num_categories; ++ i)
	{
		CL_CHECK (clGetDeviceInfo(device_id, param_names[i], param_value_size, param_value, NULL));
		std::cout << "cl device info: " << param_value << '\n';
	}
}

cl::Singleton::~Singleton()
{
	if (queue != NULL)
	{
		CL_CHECK (clReleaseCommandQueue(queue));
	}
	
	if (context)
	{
		CL_CHECK (clReleaseContext(context));
	}
}

cl_device_type cl::Singleton::GetDeviceType() const
{
	return device_type;
}

#endif
