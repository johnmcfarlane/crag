//
//  Scheduler.h
//  crag
//
//  Created by John McFarlane on 8/8/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace core
{
	template <typename BASE_CLASS, bool BITWISE_EXPANSION> class ring_buffer;
}


namespace smp
{
	// A system for performing parallelized tasks using multiple processors.
	namespace scheduler
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		// base class for task descriptor
		class Job
		{
		public:
			typedef std::size_t size_type;
			
			// virtual d'tor
			virtual ~Job() { }
			
			// Perform a unit of work which is independant of the rest,
			// i.e. one which can be performed in parallel.
			virtual void operator() (size_type unit) = 0;
		};
		
		// heterogenic collection of single-unit jobs.
		typedef core::ring_buffer<Job, true> Batch;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		// init/deinit
		void Init(size_t num_reserved_cpus);
		void Deinit();
		
		// blocking, parallel job execution
		void Complete(Job & job, size_t num_units, int priority);
		void Complete(Batch & batch, int priority);
	}
}
