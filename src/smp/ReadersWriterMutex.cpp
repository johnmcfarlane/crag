//
//  ReadersWriterMutex.cpp
//  crag
//
//  Created by John on 2011-03-17.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "ReadersWriterMutex.h"


smp::ReadersWriterMutex::ReadersWriterMutex()
: read_count(0)
, write_count(0)
{
}

void smp::ReadersWriterMutex::ReadLock()
{
	read_entry_lock.lock();
	read_lock.lock();
	if (std::atomic_fetch_add(& read_count, 1u) == 0)
	{
		write_lock.lock();
	}
	read_lock.unlock();
	read_entry_lock.unlock();
}

void smp::ReadersWriterMutex::ReadUnlock()
{
	if (std::atomic_fetch_sub(& read_count, 1u) == 1)
	{
		write_lock.unlock();
	}
}

void smp::ReadersWriterMutex::WriteLock()
{
	if (std::atomic_fetch_add(& write_count, 1u) == 0)
	{
		read_lock.lock();
	}
	
	write_lock.lock();
}

void smp::ReadersWriterMutex::WriteUnlock()
{
	write_lock.unlock();
	
	if (std::atomic_fetch_sub(& write_count, 1u) == 1)
	{
		read_lock.unlock();
	}
}
