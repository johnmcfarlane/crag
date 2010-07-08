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

#include "Node.h"


namespace
{
#include "CalculateNodeScoreGpuString.h"
}


////////////////////////////////////////////////////////////////////////////////
// CalculateNodeScoreGpuKernel members

form::CalculateNodeScoreGpuKernel::CalculateNodeScoreGpuKernel(int max_elements)
: BaseClass(kernel_source, max_elements)
, relative_camera_position(Vector4::Zero())
{
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
