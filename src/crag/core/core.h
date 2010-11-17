/*
 *  core.h
 *  crag
 *
 *  Created by John on 9/22/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */


//////////////////////////////////////////////////////////////////////
// !0

// c++0x-friendly NULL.
#define nullptr 0


//////////////////////////////////////////////////////////////////////
// OBJECT_NO_COPY
// 
// Prevent object from being copied. 
// Put at top of class definition. 
// Then try very hard not to define these functions.

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
// Then try very hard not to define these functions.

#define OBJECT_NO_INSTANCE(CLASS) \
private: \
	void operator=(const CLASS &); \
	CLASS(); \
	CLASS(const CLASS &) 
