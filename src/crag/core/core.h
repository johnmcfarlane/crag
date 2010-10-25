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
// Prevent object from being copied. Put at top of class definition. 

// Ready for C++0x
//#define OBJECT_NO_COPY(CLASS) \
//	CLASS & operator=(const CLASS&) = delete; \
//	CLASS(const CLASS&) = delete; \
//	CLASS() = default

#define OBJECT_NO_COPY(CLASS) \
private: \
	void operator=(const CLASS &); \
	CLASS(const CLASS &) 
