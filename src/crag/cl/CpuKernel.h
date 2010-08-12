/*
 *  CpuKernel.h
 *  crag
 *
 *  Created by John on 7/6/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "Kernel.h"

#if (USE_OPENCL)


namespace cl
{
	
	// A type of kernel which executes on a device.
	// Passes work to the unit in an input buffer and received results in an output buffer.
	class CpuKernel : public Kernel
	{
	public:
		CpuKernel(int max_elements, size_t element_size, void * buffer);
		
		void Process(int num_elements);
		
	private:
		virtual void SetAdditionalArgs();
		
	private:
		int max_elements;		
		size_t element_size;
		void * buffer;
		
		cl_mem buffer_object;                    // device memory used for the output array
	};
	
	
	// A templated layer for CpuKernel which deals with the input/output types to simplify its interface.
	template <typename ELEM> class CpuKernelTemplate : protected CpuKernel
	{
	public:
		CpuKernelTemplate(int init_max_elements, ELEM * init_buffer)
		: CpuKernel(init_max_elements, sizeof(ELEM), init_buffer)
		{
		}
		
		void Process(int num_elements)
		{
			CpuKernel::Process(num_elements);
		}
		
	protected:
		
		template <typename ARG> void SetAdditionalArg(int index, ARG const & value)
		{
			CpuKernel::SetAdditionalArg(index + 2, sizeof(ARG), & value);
		}
	};
	
}

#endif
