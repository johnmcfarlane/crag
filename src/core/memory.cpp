//
//  memory.cpp
//  crag
//
//  Created by John on 3/15/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "memory.h"


void * Allocate(size_t num_bytes, size_t alignment)
{
#if defined(WIN32)
	return _aligned_malloc(num_bytes, alignment);
#elif defined(__APPLE__)
	// Apple deliberately prevent use of posix_memalign. 
	// Malloc is guaranteed to be 16-byte aligned. 
	// Anything requiring greater alignment can simply run slower on mac.
	return malloc(num_bytes);
#else
	void * allocation;
	int error = posix_memalign(& allocation, 8, num_bytes);
	if (error == 0)
	{
		return allocation;
	}
	else
	{
		// EINVAL means alignement isn't max(sizeof(void*), 2^n)
		// ENOMEM means no memory
		assert(false);
		return nullptr;
	}
#endif
}

void Free(void * allocation)
{
#if ! defined(WIN32)
	return free(allocation);
#else
	return _aligned_free(allocation);
#endif
}
