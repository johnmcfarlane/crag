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


namespace smp
{
	
	// Lock is useful for critical sections and scope-base mutex locking.
	// The given mutex is locked immediately the Lock object comes into scope
	// and is unlocked when the object goes out of scope.
	
	template <typename MUTEX>
	class Lock
	{
		OBJECT_NO_COPY(Lock);
		
	public:
		Lock(MUTEX & m) : _mutex(m)
		{
			_mutex.Lock();
		}
		
		~Lock()
		{
			_mutex.Unlock();
		}
		
	private:
		MUTEX & _mutex;
	};
}
