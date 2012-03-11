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


////////////////////////////////////////////////////////////////////////////////
// Alignment-friendly allocation

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


////////////////////////////////////////////////////////////////////////////////
// Global new/delete operators for measuring the number of leaks

#if ! defined(NDEBUG) && 0

namespace
{
	int allocation_counter_throw;
	int allocation_counter_nothrow;
	int allocation_counter_throw_array;
	int allocation_counter_nothrow_array;
	bool allocation_counters_initialized = false;
}

void * operator new (std::size_t size) throw (std::bad_alloc)
{
	if (allocation_counters_initialized)
	{
		++ allocation_counter_throw;
	}
	
	return Allocate(size, std::min(16ul, size));
}
void operator delete (void* ptr) throw ()
{
	Free(ptr);
	
	if (allocation_counters_initialized)
	{
		-- allocation_counter_throw;
	}
}

void * operator new (std::size_t size, const std::nothrow_t& nothrow_constant) throw()
{
	if (allocation_counters_initialized)
	{
		++ allocation_counter_nothrow;
	}
	
	return Allocate(size, std::min(16ul, size));
}
void operator delete (void* ptr, const std::nothrow_t& nothrow_constant) throw()
{
	Free(ptr);
	
	if (allocation_counters_initialized)
	{
		-- allocation_counter_nothrow;
	}
}

void * operator new[] (std::size_t size) throw (std::bad_alloc)
{
	void * ptr = Allocate(size, std::min(16ul, size));
	
	if (allocation_counters_initialized)
	{
		++ allocation_counter_throw_array;
	}
	
	return ptr;
}
void operator delete[] (void* ptr) throw ()
{
	Free(ptr);

	if (allocation_counters_initialized)
	{
		-- allocation_counter_throw_array;
	}
}

void * operator new[] (std::size_t size, const std::nothrow_t& nothrow_constant) throw()
{
	if (allocation_counters_initialized)
	{
		++ allocation_counter_nothrow_array;
	}
	
	return Allocate(size, std::min(16ul, size));
}
void operator delete[] (void* ptr, const std::nothrow_t& nothrow_constant) throw()
{
	Free(ptr);
	
	if (allocation_counters_initialized)
	{
		-- allocation_counter_nothrow_array;
	}
}

void InitAllocationCounters()
{
	ASSERT(! allocation_counters_initialized);
	allocation_counter_throw = 0;
	allocation_counter_nothrow = 0;
	allocation_counter_throw_array = 0;
	allocation_counter_nothrow_array = 0;
	allocation_counters_initialized = true;
}

void DeinitAllocationCounters()
{
	ASSERT(allocation_counters_initialized);
	ASSERT(allocation_counter_throw == 0);
	ASSERT(allocation_counter_nothrow == 0);
	ASSERT(allocation_counter_throw_array == 0);
	ASSERT(allocation_counter_nothrow_array == 0);
	allocation_counters_initialized = false;
}

#else

void InitAllocationCounters()
{
}

void DeinitAllocationCounters()
{
}

#endif
