/*
 *  Lock.h
 *  crag
 *
 *  Created by John on 10/14/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Mutex.h"


namespace smp
{
	
	// Simple mutex lock scoper.
	// This might be correct terminology.
	
	class Lock
	{
		OBJECT_NO_COPY(Lock);
		
	public:
		Lock(Mutex & m) : mutex(m)
		{
			mutex.Lock();
		}
		
		~Lock()
		{
			mutex.Unlock();
		}
		
	private:
		Mutex & mutex;
	};
}
