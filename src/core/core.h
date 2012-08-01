//
//  core.h
//  crag
//
//  Created by John on 9/22/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


//////////////////////////////////////////////////////////////////////
// OBJECT_NO_COPY
// 
// Prevent object from being copied. 
// Put at top of class definition. 

// Ready for C++0x
#define OBJECT_NO_COPY(CLASS) \
	CLASS & operator=(const CLASS&) = delete; \
	CLASS(const CLASS&) = delete;


//////////////////////////////////////////////////////////////////////
// OBJECT_NO_INSTANCE
// 
// Prevent object from being instanced. 
// Put at top of class definition. 

#define OBJECT_NO_INSTANCE(CLASS) \
	OBJECT_NO_COPY(CLASS); \
	CLASS()


//////////////////////////////////////////////////////////////////////
// OBJECT_SINGLETON
// 
// Prevent object from being instanced more than once.
// Put at top of class definition; include "core/Singleton.h".

#if defined(NDEBUG)
#define OBJECT_SINGLETON(CLASS) \
	OBJECT_NO_COPY(CLASS)
#else
#define OBJECT_SINGLETON(CLASS) \
	OBJECT_NO_COPY(CLASS); \
private: \
    core::Singleton<CLASS> _singleton
#endif


//////////////////////////////////////////////////////////////////////
// get_owner - inverse pointer to member

namespace core
{
	// Given an instance of TYPE which is contained in CLASS as MEMBER,
	// returns the containing object of CLASS.
	template <typename CLASS, typename TYPE, TYPE CLASS::*MEMBER>
	CLASS & get_owner(TYPE & constituent)
	{
		// Get byte pointer to constituent.
		char * hook_cptr = (char *)& constituent;
		
		// Get byte counter to where constituent would be in a null object.
		CLASS * const null_object_ptr = nullptr;
		char * null_hook_cptr = (char *)&(null_object_ptr->*MEMBER);
		
		// Get the difference between the two pointers.
		// This is equal to the byte pointer of the object of which h is a part.
		// (Hopefully, this is the first line that actually makes machine code.)
		size_t owner_offset = hook_cptr - null_hook_cptr;
		
		// Cast back to type CLASS.
		CLASS & owner = * reinterpret_cast<CLASS *>(owner_offset);
		
		// Sanity check - go back the other way.
		assert(& (owner.*MEMBER) == & constituent);
		
		return owner;
	}
}


//////////////////////////////////////////////////////////////////////
// core::raw_type - strips const, ref and pointer from types

namespace core
{
	template <typename T> struct raw_type;
	template<typename T> struct raw_type			{ typedef T type; };
	template<typename T> struct raw_type<const T>	{ typedef typename raw_type<T>::type type; };
	template<typename T> struct raw_type<T &>		{ typedef typename raw_type<T>::type type; };
	template<typename T> struct raw_type<T *>		{ typedef typename raw_type<T>::type type; };
}

//////////////////////////////////////////////////////////////////////
// Time - in seconds

typedef double Time;
