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
{
	cl_int err;
	
	// Get an ID for the device
	CL_CHECK (clGetDeviceIDs(NULL, device_type, 1, &device_id, NULL));
	
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
