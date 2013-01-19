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

////////////////////////////////////////////////////////////////////////////////
// Page Allocations

// size of system page size
size_t RoundToPageSize(size_t num_bytes);
size_t GetPageSize();

void * AllocatePage(size_t num_bytes);
void FreePage(void * allocation, size_t num_bytes);

////////////////////////////////////////////////////////////////////////////////
// Aligned Allocation

void * Allocate(size_t num_bytes, size_t alignment = sizeof(void *));
void Free(void * allocation);

template <typename T>
T* Allocate(size_t count)
{
	size_t num_bytes = count * sizeof(T);
	size_t alignment = std::max(alignof(T), sizeof(void*));
	void * buffer = Allocate(num_bytes, alignment);
	return reinterpret_cast<T *>(buffer);
}

void InitAllocationCounters();
void DeinitAllocationCounters();

////////////////////////////////////////////////////////////////////////////////
// Object allocation macros

// DECLARE_ALLOCATOR:
//  placed in class definition, then add one of the three DEFINE macros: 
//  DEFINE_DEFAULT_ALLOCATOR, DISABLE_ALLOCATOR or DEFINE_POOL_ALLOCATOR 
//  in class' compilation unit.
#define DECLARE_ALLOCATOR(TYPE) \
	void* operator new(size_t sz); \
	void* operator new [](size_t sz) throw(); \
	void operator delete(void* p); \
	void operator delete [](void* p);

// regular, do-nothing implementation;
// placed with class member definitions
#define DEFINE_DEFAULT_ALLOCATOR(TYPE) \
	void* TYPE::operator new(size_t sz) \
	{ \
		ASSERT(sz == sizeof(TYPE)); \
		return ::Allocate(sz, alignof(TYPE)); \
	} \
	void* TYPE::operator new [](size_t sz) throw() \
	{ \
		ASSERT(sz == sizeof(TYPE)); \
		return ::Allocate(sz, alignof(TYPE)); \
	} \
	void TYPE::operator delete(void* p) \
	{ \
		::Free(p); \
	} \
	void TYPE::operator delete [](void* p) \
	{ \
		::Free(p); \
	}

// prevents class (or sub-classes) from being allocated;
// placed with class member definitions
#define DISABLE_ALLOCATOR(TYPE) \
	void* TYPE::operator new(size_t sz) \
	{ \
		DEBUG_BREAK("disabled"); \
		return ::Allocate(sz, alignof(TYPE)); \
	} \
	void* TYPE::operator new [](size_t sz) throw() \
	{ \
		DEBUG_BREAK("disabled"); \
		return ::Allocate(sz, alignof(TYPE)); \
	} \
	void TYPE::operator delete(void* p) \
	{ \
		::Free(p); \
	} \
	void TYPE::operator delete [](void* p) \
	{ \
		::Free(p); \
	}
