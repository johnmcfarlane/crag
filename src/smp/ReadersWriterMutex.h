//
//  ReadersWriterMutex.h
//  crag
//
//  Created by John on 2011-03-17.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "SimpleMutex.h"

namespace smp
{
	
	// An implementation of a Readers/Writers lock mutex.
	// Maintains mutual exclusivity for writing to a resource
	// but allows unlimited reading.
	// source: http://www.thegrumpyprogrammer.com/node/16
	
	class ReadersWriterMutex
	{
		OBJECT_NO_COPY(ReadersWriterMutex);
		
	public:
		ReadersWriterMutex();
		
		void ReadLock();
		void ReadUnlock();
		
		void WriteLock();
		void WriteUnlock();
		
	private:
		std::atomic<unsigned> read_count;
		std::atomic<unsigned> write_count;
		
		SimpleMutex read_entry_lock;
		std::mutex write_lock;
		std::mutex read_lock;
	};
	
}
