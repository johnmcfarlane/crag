//
//  Scheduler.h
//  crag
//
//  Created by John McFarlane on 8/8/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace smp
{
	// A system for performing parallelized tasks using multiple processors.
	namespace scheduler
	{
		// init/deinit
		void Init();
		void Deinit();
		
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

		// blocking, parallel job execution
		void Complete(Job & job, int num_units, int priority);
	}
}
