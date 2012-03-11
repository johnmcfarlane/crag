//
//  memory.h
//  crag
//
//  Created by John on 11/8/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#if ! defined(CRAG_PCH)
#error pch.h not included.
#endif


// This definition indicates whether stack frame addresses increase or decrease as the stack grows.
#if defined(__ppc__) || defined(__ppc64__) || defined(__i386__) || defined(__x86_64__) || defined(WIN32)
#define STACK_GROWTH_DIRECTION -1
#endif

// This definition is highly questionable. Double check it works if compiling on a new platform.
// I'm assuming that addresses don't need to be aligned for calls to __builtin_prefetch.
#if defined(__ppc__)
#define CACHE_LINE_WIDTH 32
#elif defined(__ppc64__) || defined(__i386__) || defined(__x86_64__)
#define CACHE_LINE_WIDTH 128
#elif defined(WIN32)
#define CACHE_LINE_WIDTH 128
#include <xmmintrin.h>
#else
#error Unidentified platform.
#endif

// Array size
#define ARRAY_SIZE(ARRAY) (sizeof(ARRAY)/sizeof(*ARRAY))


//////////////////////////////////////////////////////////////////////
// Low-level Memory Manipulation using templated parameters

// ZeroMemory

#if ! defined(WIN32)
inline void ZeroMemory(char * ptr, size_t num_bytes)
{
	memset(static_cast<void *>(ptr), 0, num_bytes);
}
#endif

template<typename T> inline void ZeroArray(T * object_ptr, int count)
{
	ZeroMemory(reinterpret_cast<char *>(object_ptr), sizeof(T) * count);
}

template<typename T> inline void ZeroObject(T & object)
{
	ZeroMemory(reinterpret_cast<char *>(& object), sizeof(object));
}


// Bitwise Copy

template<typename T> inline void BitwiseCopyArray(T * lhs, T const * rhs, int count)
{
	// Make sure this isn't a job for memmove.
	assert(Abs(lhs - rhs) >= count);
	
	size_t num_bytes = sizeof(T) * count;
	
	memcpy(lhs, rhs, num_bytes);
}

template<typename T> inline void BitwiseCopyObject(T & lhs, T const & rhs)
{
	BitwiseCopyArray(& lhs, & rhs, 1);
}


// Bitwise Swap

template<typename T> inline void BitwiseSwapObject(T & lhs, T & rhs)
{
	char buffer [sizeof (T)];
	T & tmp = * reinterpret_cast<T *>(buffer);
	BitwiseCopyObject(tmp, lhs);
	BitwiseCopyObject(lhs, rhs);
	BitwiseCopyObject(rhs, tmp);
}


// Prefetch

inline void PrefetchBlock(void const * ptr)
{
#if defined(__GNUC__)
	__builtin_prefetch(ptr);
#elif defined(WIN32)
	_mm_prefetch(reinterpret_cast<char const *>(ptr), _MM_HINT_T0);
#else
#endif
}

inline void PrefetchMemory(char const * begin, char const * end)
{
	// End still means _after_ the last item.
	// However, a zero-sized prefetch is wasteful enough that it is prohibited.
	assert(end > begin);
	
	do
	{
		PrefetchBlock(begin);
		begin += CACHE_LINE_WIDTH;
	}	while (begin < end);
}

// Iterator should be random access for this algorithm to work efficiently.
template<typename ITERATOR> inline void PrefetchArray(ITERATOR begin, ITERATOR end)
{
	PrefetchMemory(& reinterpret_cast<char const &>(* begin), & reinterpret_cast<char const &>(* end));
}

template<typename T> inline void PrefetchArray(T const * object_ptr, int count)
{
	PrefetchArray(object_ptr, object_ptr + count);
}

template<typename T> inline void PrefetchObject(T const & object)
{
	PrefetchArray((& object), (& object) + 1);
}


////////////////////////////////////////////////////////////////////////////////
// Aligned Allocation

// Given an element of size, type_size, 
// calculate the optimum alignment for array allocation.
inline size_t CalculateAlignment(size_t type_size)
{
	size_t alignment = 1;
	while ((type_size & 1) == 0)
	{
		alignment <<= 1;
		type_size >>= 1;
	}
	return alignment;
}

void * Allocate(size_t num_bytes, size_t alignment = sizeof(void *));
void Free(void * allocation);

void InitAllocationCounters();
void DeinitAllocationCounters();
