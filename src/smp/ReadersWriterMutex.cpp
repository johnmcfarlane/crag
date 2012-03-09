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

#include "atomic.h"


smp::ReadersWriterMutex::ReadersWriterMutex()
: read_count(0)
, write_count(0)
{
}

void smp::ReadersWriterMutex::ReadLock()
{
	read_entry_lock.Lock();
	read_lock.Lock();
	if (AtomicFetchAndAdd(read_count, 1) == 0)
	{
		write_lock.Lock();
	}
	read_lock.Unlock();
	read_entry_lock.Unlock();
}

void smp::ReadersWriterMutex::ReadUnlock()
{
	if (AtomicFetchAndAdd(read_count, -1) == 1)
	{
		write_lock.Unlock();				
	}
}

void smp::ReadersWriterMutex::WriteLock()
{
	if (AtomicFetchAndAdd(write_count, 1) == 0)
	{
		read_lock.Lock();
	}
	
	write_lock.Lock();
}

void smp::ReadersWriterMutex::WriteUnlock()
{
	write_lock.Unlock();
	
	if (AtomicFetchAndAdd(write_count, -1) == 1) 
	{
		read_lock.Unlock();	
	}
}
