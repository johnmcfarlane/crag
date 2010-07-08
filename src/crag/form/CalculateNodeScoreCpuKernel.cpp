/*
 *  CalculateNodeScoreKernel.cpp
 *  crag
 *
 *  Created by John on 7/5/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "CalculateNodeScoreCpuKernel.h"

#if (USE_OPENCL)

#include "Node.h"


namespace
{
#include "CalculateNodeScoreCpuString.h"
}


////////////////////////////////////////////////////////////////////////////////
// CalculateNodeScoreCpuKernel members

form::CalculateNodeScoreCpuKernel::CalculateNodeScoreCpuKernel(int max_elements, Node * nodes)
: BaseClass(kernel_source, max_elements, nodes)
, relative_camera_position(Vector4::Zero())
{
}

void form::CalculateNodeScoreCpuKernel::Process(Node * nodes, Node const * nodes_end, Vector3 const & init_relative_camera_position)
{
	relative_camera_position.x = init_relative_camera_position.x;
	relative_camera_position.y = init_relative_camera_position.y;
	relative_camera_position.z = init_relative_camera_position.z;
	
	BaseClass::Process(nodes_end - nodes);
}

void form::CalculateNodeScoreCpuKernel::SetAdditionalArgs()
{
	BaseClass::SetAdditionalArg(0, relative_camera_position);
}

#endif
