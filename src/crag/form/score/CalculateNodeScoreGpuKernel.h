/*
 *  CalculateNodeScoreGpuKernel.h
 *  crag
 *
 *  Created by John on 7/7/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */


#include "cl/GpuKernel.h"

#if defined(USE_OPENCL)

#include "form/defs.h"


namespace form
{
	// forward-declare
	class Node;
	
	
	// Helper for CalculateNodeScoreGpuKernel;
	// Refines a node down the the information necessary to score it.
	struct CalculateNodeScoreInput 
	{
		Vector3 center;
		float area;
	};
	
	// CL Kernel for calculating node scores on the GPU.
	class CalculateNodeScoreGpuKernel : public cl::GpuKernelTemplate<CalculateNodeScoreInput, float>
	{
		typedef cl::GpuKernelTemplate<CalculateNodeScoreInput, float> BaseClass;
	public:
		CalculateNodeScoreGpuKernel(int max_elements);
		
		void Process(Node * nodes, Node const * nodes_end, Vector3 const & relative_camera_position);
		
	private:
		void SetAdditionalArgs();
		
		Vector4 relative_camera_position;
	};
}

#endif
