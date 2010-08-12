/*
 *  CalculateNodeScoreGpuKernel.cpp
 *  crag
 *
 *  Created by John on 7/7/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */
#include "pch.h"

#include "CalculateNodeScoreGpuKernel.h"

#if (USE_OPENCL)

#include "form/node/Node.h"


namespace
{
#include "CalculateNodeScoreGpuString.h"
	
#if defined(PROFILE)
	char const * gpu_kernel_cl_filename = "src/crag/form/score/CalculateNodeScoreGpu.cl";
	char const * gpu_kernel_cpp_filename = "src/crag/form/score/CalculateNodeScoreGpuString.h";
#endif
}


////////////////////////////////////////////////////////////////////////////////
// CalculateNodeScoreGpuKernel members

form::CalculateNodeScoreGpuKernel::CalculateNodeScoreGpuKernel(int init_max_elements)
: BaseClass(init_max_elements)
, relative_camera_position(Vector4::Zero())
{
#if defined(PROFILE)
	// TODO: Check to see if output file has actually changed. If so, exit program.
	char * cl_source_string = cl::Kernel::LoadClFile(gpu_kernel_cl_filename);
	
	if (Compile(cl_source_string))
	{
		if (strcmp(cl_source_string, kernel_source) != 0)
		{
			cl::Kernel::SaveCFile(gpu_kernel_cpp_filename, cl_source_string, "kernel_source");
		}
	}
	else
	{
		Compile(kernel_source);
	}
	
	delete [] cl_source_string;
#else
	Compile(kernel_source);
#endif
}

void form::CalculateNodeScoreGpuKernel::Process(Node * nodes, Node const * nodes_end, Vector3 const & init_relative_camera_position)
{
	relative_camera_position.x = init_relative_camera_position.x;
	relative_camera_position.y = init_relative_camera_position.y;
	relative_camera_position.z = init_relative_camera_position.z;
	
	Node * node_iterator;
	
	// Copy score-relevant data from nodes to kernel_inputs.
	CalculateNodeScoreInput * input_iterator = input_data;
	node_iterator = nodes;
	while (node_iterator != nodes_end)
	{
		input_iterator->center = node_iterator->center;
		input_iterator->area = node_iterator->area;
		++ input_iterator;
		++ node_iterator;
	}
	
	BaseClass::Process(nodes_end - nodes);
	
	float * output_iterator = output_data;
	node_iterator = nodes;
	while ( node_iterator != nodes_end)
	{
		node_iterator->score = * output_iterator;
		++ output_iterator;
		++ node_iterator;
	}
}

void form::CalculateNodeScoreGpuKernel::SetAdditionalArgs()
{
	BaseClass::SetAdditionalArg(0, relative_camera_position);
}

#endif
