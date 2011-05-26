/*
 *  core.h
 *  crag
 *
 *  Created by John on 9/22/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


//////////////////////////////////////////////////////////////////////
// !0

// c++0x-friendly NULL.
#define nullptr 0


//////////////////////////////////////////////////////////////////////
// static_assert

// provides some of the functionality of c++0x compile-time asserts.
#define static_assert(pred, message) switch(0){case 0:case pred:;}


//////////////////////////////////////////////////////////////////////
// OBJECT_NO_COPY
// 
// Prevent object from being copied. 
// Put at top of class definition. 

// Ready for C++0x
//#define OBJECT_NO_COPY(CLASS) \
//	CLASS & operator=(const CLASS&) = delete; \
//	CLASS(const CLASS&) = delete; \
//	CLASS() = default

#define OBJECT_NO_COPY(CLASS) \
private: \
	void operator=(const CLASS &); \
	CLASS(const CLASS &) 


//////////////////////////////////////////////////////////////////////
// OBJECT_NO_INSTANCE
// 
// Prevent object from being instanced. 
// Put at top of class definition. 

#define OBJECT_NO_INSTANCE(CLASS) \
private: \
	void operator=(const CLASS &); \
	CLASS(); \
	CLASS(const CLASS &) 


//////////////////////////////////////////////////////////////////////
// OBJECT_SINGLETON
// 
// Prevent object from being instanced more than once.
// Put at top of class definition. 

#if defined(NDEBUG)
#define OBJECT_SINGLETON(CLASS) \
private: \
    enum { _is_singleton = 1; }
#else
#define OBJECT_SINGLETON(CLASS) \
private: \
    core::Singleton<CLASS> _singleton
#endif
