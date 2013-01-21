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

#if ! defined(WIN32)
#include <sys/mman.h>
#include <unistd.h>
#endif

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
	int error = posix_memalign(& allocation, alignment, num_bytes);
	switch (error)
	{
	case 0:
		return allocation;
		
	case EINVAL:
		DEBUG_BREAK("alignment, " SIZE_T_FORMAT_SPEC ", is not a power of two, or is less than minimum, " SIZE_T_FORMAT_SPEC, alignment, sizeof(void*));
		break;
		
	case ENOMEM:
		break;
		
	default:
		DEBUG_BREAK("bad value returned by posix_memalign");
	}
	
	return nullptr;
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
// Page Alignment - large chunks of memory

size_t RoundToPageSize(size_t required_bytes)
{
	size_t page_mask = GetPageSize() - 1;
	return (required_bytes + (page_mask - 1)) & ~ page_mask;
}

size_t GetPageSize()
{
	static size_t page_size = 0;
	if (page_size == 0)
	{
#if defined(WIN32)
		SYSTEM_INFO system_info;
		GetSystemInfo(& system_info);
		page_size = system_info.dwPageSize;
#else
		page_size = sysconf (_SC_PAGE_SIZE);
#endif
	}
	ASSERT(page_size > 0);

	return page_size;
}

void * AllocatePage(size_t num_bytes)
{
	// check allocation size is rounded up correctly
	ASSERT((num_bytes & (GetPageSize() - 1)) == 0);

	// allocate
#if defined(WIN32)
	void * p = VirtualAlloc(nullptr, num_bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
	void * p = mmap(nullptr, num_bytes, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#endif

	if (p == nullptr)
	{
#if defined(WIN32)
		DEBUG_BREAK("AllocatePage(0, " SIZE_T_FORMAT_SPEC ", ...) failed with error code, %X", num_bytes, GetLastError());
#else
		DEBUG_BREAK("mmap(..., " SIZE_T_FORMAT_SPEC ", ...) failed with error code, %X", num_bytes, errno);
#endif
	}

	return p;
}

void FreePage(void * allocation, size_t num_bytes)
{
	ASSERT((reinterpret_cast<uintptr_t>(allocation) & (GetPageSize() - 1)) == 0);

#if defined(WIN32)
	if (! VirtualFree(allocation, 0, MEM_RELEASE))
	{
		DEBUG_BREAK("VirtualFree(%p, 0, MEM_RELEASE) failed with error code, %X", allocation, GetLastError());
	}
#else
	if (munmap(allocation, num_bytes) != 0)
	{
		DEBUG_BREAK("munmap(%p, " SIZE_T_FORMAT_SPEC ") failed with error code, %d", allocation, num_bytes, errno);
	}
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
