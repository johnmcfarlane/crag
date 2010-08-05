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

#include "form/node/Node.h"


namespace
{
#include "CalculateNodeScoreCpuString.h"
}


////////////////////////////////////////////////////////////////////////////////
// CalculateNodeScoreCpuKernel members

form::CalculateNodeScoreCpuKernel::CalculateNodeScoreCpuKernel(int init_max_elements, Node * nodes)
: BaseClass(kernel_source, init_max_elements, nodes)
, camera_position(Vector4::Zero())
, camera_direction(Vector4::Zero())
{
}

void form::CalculateNodeScoreCpuKernel::Process(Node * nodes, 
												Node const * nodes_end, 
												Vector3 const & init_camera_position, 
												Vector3 const & init_camera_direction)
{
	camera_position.x = init_camera_position.x;
	camera_position.y = init_camera_position.y;
	camera_position.z = init_camera_position.z;
	
	camera_direction.x = init_camera_direction.x;
	camera_direction.y = init_camera_direction.y;
	camera_direction.z = init_camera_direction.z;
	
	BaseClass::Process(nodes_end - nodes);
}

void form::CalculateNodeScoreCpuKernel::SetAdditionalArgs()
{
	BaseClass::SetAdditionalArg(0, camera_position);
	BaseClass::SetAdditionalArg(1, camera_direction);
}

#endif
