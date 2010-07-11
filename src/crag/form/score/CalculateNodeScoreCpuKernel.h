/*
 *  CalculateNodeScoreKernel.h
 *  crag
 *
 *  Created by John on 7/5/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */


#include "cl/CpuKernel.h"

#if (USE_OPENCL)

#include "form/defs.h"


namespace form
{
	// forward-declare
	class Node;
	
	
	// CL Kernel for calculating node scores on the CPU.
	class CalculateNodeScoreCpuKernel : public cl::CpuKernelTemplate<Node>
	{
		typedef cl::CpuKernelTemplate<Node> BaseClass;
	public:
		CalculateNodeScoreCpuKernel(int max_elements, Node * nodes);
		
		void Process(Node * nodes, Node const * nodes_end, Vector3 const & relative_camera_position);
		
	private:
		void SetAdditionalArgs();
		
		Vector4 relative_camera_position;
	};
	
}

#endif
