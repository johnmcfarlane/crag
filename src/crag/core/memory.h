/*
 *  MemoryUtils.h
 *  Crag
 *
 *  Created by John on 11/8/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#if ! defined(CRAG_PCH)
#error pch.h not included.
#endif


//////////////////////////////////////////////////////////////////////
// Memory Wrecking

// This definition is highly questionable. Doublt check it work if compiling on a new platform.
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

// Copy
template<typename T> inline void CopyArray(T * lhs, T const * rhs, int count)
{
	// Make sure this isn't a job for memmove.
	assert(abs(lhs - rhs) >= count);
	
	size_t num_bytes = sizeof(T) * count;
	
	memcpy(lhs, rhs, num_bytes);
}

template<typename T> inline void CopyObject(T & lhs, T const & rhs)
{
	CopyArray(& lhs, & rhs, 1);
}


////////////////////////////////////////////////////////////////////////////////
// Prefetch

inline void PrefetchBlock(void const * ptr)
{
#if defined(__GNUC__)
	__builtin_prefetch(ptr);
#elif defined(WIN32)
	_mm_prefetch(reinterpret_cast<char const *>(ptr), _MM_HINT_T0);	// or is _MM_HINT_NTA better?
#else
#endif
}

inline void PrefetchMemory(char const * begin, char const * end)
{
	assert(end > begin);
	
	do
	{
		PrefetchBlock(begin);
		begin += CACHE_LINE_WIDTH;
	}	while (begin < end);
}

template<typename T> inline void PrefetchArray(T const * begin, T const * end)
{
	PrefetchMemory(reinterpret_cast<char const *>(begin), reinterpret_cast<char const *>(end));
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
// ForEach with Prefetch

template <class FUNCTOR, class ELEMENT> void ForEach(FUNCTOR & f, ELEMENT * begin, ELEMENT * end, int step_size)
{
	int total_num_nodes = end - begin;
	int full_steps = total_num_nodes / step_size;
	
	// main pass
	for (int step = 0; step < full_steps; step ++)
	{
		ELEMENT * sub_begin = begin + step * step_size;
		ELEMENT * sub_end = sub_begin + step_size;
		ForEach_Sub(f, sub_begin, sub_end);
	}
	
	// remainder
	ELEMENT * sub_begin = begin + full_steps * step_size;
	ELEMENT * sub_end = end;
	Assert(sub_end - sub_begin == total_num_nodes % step_size);
	
	if (sub_begin < sub_end)
	{
		ForEach_Sub(f, sub_begin, sub_end);
	}
}

template <class FUNCTOR, class ELEMENT> void ForEach_Sub(FUNCTOR & f, ELEMENT * begin, ELEMENT * end)
{
	// Pre-fetch the actual nodes.
	// TODO: Find out if this does any good at all. 
	PrefetchArray(begin, end);
	
	// Do any additional pre-fetching desired by the functor.
	if (f.PerformPrefetchPass()) {
		for (ELEMENT * iterator = begin; iterator != end; ++ iterator) {
			f.OnPrefetchPass(* iterator);
		}
	}
	
	// Now do the work.
	for (ELEMENT * iterator = begin; iterator != end; ++ iterator) {
		f(* iterator);
	}
}


////////////////////////////////////////////////////////////////////////////////
// Aligned Allocation

inline void * Allocate(size_t num_bytes, size_t alignment = sizeof(void *))
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

inline void Free(void * allocation)
{
#if ! defined(WIN32)
	return free(allocation);
#else
	return _aligned_free(allocation);
#endif
}


////////////////////////////////////////////////////////////////////////////////
// Global new/delete

/*inline void * operator new (std::size_t num_bytes, const std::nothrow_t&) throw()
{
	return Allocate(num_bytes);
}

inline void * operator new [] (std::size_t num_bytes, const std::nothrow_t&) throw()
{
	return Allocate(num_bytes);
}

inline void operator delete(void * ptr) throw()
{
	Free(ptr);
}*/


////////////////////////////////////////////////////////////////////////////////
// Class new/delete

// This only dictates the alignment of the first element (when allocating an array).
#define OVERLOAD_NEW_DELETE(ALIGNMENT) \
	void * operator new (size_t num_bytes) { return Allocate(num_bytes, ALIGNMENT); } \
	void * operator new [] (size_t num_bytes) { return Allocate(num_bytes, ALIGNMENT); } \
	void operator delete (void * ptr) { Free(ptr); }
