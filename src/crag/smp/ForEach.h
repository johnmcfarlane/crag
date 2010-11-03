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


namespace smp
{
	void Init(int num_reserved_cores = 0);
	void Deinit();
	
	// Overload this functor ...
	class Functor
	{
	public:
		virtual ~Functor() { }
		virtual void operator () (int first, int last) = 0;
	};
	
	// ... and pass it in here. 
	// Function won't return until all work is done.
	void ForEach(int first, int last, int step, Functor & f);
}
