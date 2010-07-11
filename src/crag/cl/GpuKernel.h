/*
 *  GpuKernel.h
 *  crag
 *
 *  Created by John on 7/6/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "defs.h"

#if (USE_OPENCL)

#include "Kernel.h"


namespace cl
{
	
	// A type of kernel which executes on a device.
	// Passes work to the unit in an input buffer and received results in an output buffer.
	class GpuKernel : public Kernel
	{
	public:
		GpuKernel(char const * kernel_source, int max_elements, size_t input_size, size_t output_size);
		~GpuKernel();
		
		void Process(void const * input_data, void * output_data, int num_elements);
		
	private:
		virtual void SetAdditionalArgs();
		
	private:
		int max_elements;		
		size_t input_size, output_size;
		
		cl_mem input;                     // device memory used for the input array
		cl_mem output;                    // device memory used for the output array
	};
	
	
	// A templated layer for GpuKernel which deals with the input/output types to simplify its interface.
	template <typename IN, typename OUT> class GpuKernelTemplate : protected GpuKernel
	{
	public:
		GpuKernelTemplate(char const * kernel_source, int init_max_elements)
		: GpuKernel(kernel_source, init_max_elements, sizeof(IN), sizeof(OUT))
		, input_data(new IN [init_max_elements])
		, output_data(new OUT [init_max_elements])
		{
		}
		
		~GpuKernelTemplate()
		{
			delete [] output_data;
			delete [] input_data;
		}
		
		void Process(int num_elements)
		{
			GpuKernel::Process(input_data, output_data, num_elements);
		}
		
	protected:
		
		template <typename ARG> void SetAdditionalArg(int index, ARG const & value)
		{
			GpuKernel::SetAdditionalArg(index + 3, sizeof(ARG), & value);
		}
		
		IN * input_data;
		OUT * output_data;
	};
	
}

#endif
