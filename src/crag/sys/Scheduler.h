/*
 *  Scheduler.h
 *  crag
 *
 *  Created by John on 8/23/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "core/Singleton.h"


namespace sys
{

	// Takes care dishing out work to multiple CPUs with (hopefully) some efficiency.
	// Think of it as a substitute for parallel_for or certain OpenMP pragmas.
	class Scheduler : public core::Singleton<Scheduler>
	{
	public:
		Scheduler(int num_reserved_cpus = 0);
		~Scheduler();
		
		// Overload this functor ...
		class Functor
		{
		public:
			virtual ~Functor() { }
			virtual void operator () (int first, int last) = 0;
		};
		
		// ... and pass it in here.
		// TODO: Add interleaved version.
		void Dispatch(Functor & f, int first, int last, int step);
		
	private:
		void DispatchSub(Functor & f, int first, int last);

		// TODO: Add asynchronous option and make this public. 
		bool IsComplete() const;

		class Task;
		class Slot;
		
		int num_slots;
		Slot * slots;
	};
	
}
