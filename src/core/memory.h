//
//  memory.h
//  crag
//
//  Created by John on 11/8/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

// Array size
#define ARRAY_SIZE(ARRAY) extent <decltype(ARRAY)>::value;

////////////////////////////////////////////////////////////////////////////////
// heap error checking

void DebugCheckMemory(int line, char const * filename);

#if defined(CRAG_RELEASE)
#define CRAG_DEBUG_CHECK_MEMORY() DO_NOTHING
#else
#define CRAG_DEBUG_CHECK_MEMORY() DebugCheckMemory(__LINE__, __FILE__)
#endif


//////////////////////////////////////////////////////////////////////
// Low-level Memory Manipulation using templated parameters

// ZeroMemory

inline void ZeroMemory(char * ptr, int num_bytes)
{
	memset(static_cast<void *>(ptr), 0, num_bytes);
}

template<typename T> inline void ZeroArray(T * object_ptr, int count)
{
	ZeroMemory(reinterpret_cast<char *>(object_ptr), sizeof(T) * count);
}

template<typename T> inline void ZeroObject(T & object)
{
	ZeroMemory(reinterpret_cast<char *>(& object), sizeof(object));
}

////////////////////////////////////////////////////////////////////////////////
// Alignment

namespace crag
{
	namespace core
	{
		template <typename T>
		bool IsAligned(T const * ptr, int alignment)
		{
			ASSERT(alignment > 0);

			auto address_bytes = reinterpret_cast<std::intptr_t>(ptr);
			return ! (address_bytes & (alignment - 1));
		}

		template <typename T>
		bool IsAligned(T const * ptr)
		{
			return IsAligned<T>(ptr, alignof(T));
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Aligned Allocation

void * Allocate(int num_bytes, int alignment = sizeof(void *));
void Free(void * allocation);
void CheckMemory();

////////////////////////////////////////////////////////////////////////////////
// Page Allocations

// size of system page size
int RoundToPageSize(int num_bytes);
int GetPageSize();

void * AllocatePage(int num_bytes);
void FreePage(void * allocation, int num_bytes);
