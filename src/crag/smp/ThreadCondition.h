/*
 *  ThreadCondition.h
 *  crag
 *
 *  Created by John on 9/24/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "sys/App.h"


namespace smp
{
	
	// A thread 'condition' is kind-of a gate which allows one thread to dictate when another thread can continue.
	// It's straight out of SDL which has limited documentation on the subject.
	
	class ThreadCondition 
	{
		OBJECT_NO_COPY(ThreadCondition);
	public:
		
		ThreadCondition ();
		~ThreadCondition();
		
		void Wait(class Mutex & mutex);
		void Wait(class Mutex & mutex, sys::TimeType timeout);
		
		void Restart();	// Restarts a caller of Wait
		void RestartAll();	// Restarts all callers of Wait
		
	private:
		
		SDL_cond * sdl_condition;
	};
	
}
