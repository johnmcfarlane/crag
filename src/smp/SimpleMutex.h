//
//  SimpleMutex.h
//  crag
//
//  Created by John on 8/26/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Mutex.h"


namespace smp
{
	
	// Wrapper for SDL implementation of a spin lock. 
	// Only use this for brief locks. 

	class SimpleMutex
	{
		OBJECT_NO_COPY(SimpleMutex);
		
	public:
		SimpleMutex()
		: spin_lock(0)
		{
		}
		
		void Lock()
		{
			AssertErrno();
			SDL_AtomicLock(& spin_lock);
			errno = 0;
		}
		
		void Unlock()
		{
			SDL_AtomicUnlock(& spin_lock);
		}
		
	private:
		SDL_SpinLock spin_lock;
	};

}
