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

#if defined(WIN32)
#define OBJECT_NO_COPY(CLASS) \
	CLASS & operator=(const CLASS&); \
	CLASS(const CLASS&);

#else
#define OBJECT_NO_COPY(CLASS) \
	CLASS & operator=(const CLASS&) = delete; \
	CLASS(const CLASS&) = delete;
#endif


//////////////////////////////////////////////////////////////////////
// Missing keywords etc.

#if defined(WIN32)
#define constexpr const
#define alignof __alignof
#define noexcept _NOEXCEPT
#endif


//////////////////////////////////////////////////////////////////////
// Optimization directives

// EXPECT - tells compiler to expect EXP==C; returns EXP
#if defined(__GNUC__)
#define ASSUME(CONDITION) __builtin_expect(CONDITION, true)
#elif defined(WIN32)
#define ASSUME(CONDITION) __assume(CONDITION)
#else
#define ASSUME(CONDITION) (CONDITION)
#endif

// UNREACHABLE - tells the compiler, you can't get here
#if defined(__GNUC__) || defined(__clang__)
#define UNREACHABLE __builtin_unreachable
#else
#define UNREACHABLE() __assume(false)
#endif


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

namespace core
{
	//////////////////////////////////////////////////////////////////////
	// get_owner - inverse pointer to member

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

	//////////////////////////////////////////////////////////////////////
	// Time - in seconds

	typedef double Time;

	// converts from std::chrono::duration to Time
	template <typename DURATION>
	Time DurationToSeconds(DURATION duration)
	{
		typedef typename DURATION::period period;
	
		constexpr auto scale = Time(period::num) / period::den;
		auto count = duration.count();
	
		Time seconds = Time(count) * scale;
		return seconds;
	}

	// converts from Time to std::chrono::duration
	template <typename DURATION>
	DURATION SecondsToDuration(Time seconds)
	{
		typedef typename DURATION::period period;
		typedef typename DURATION::rep rep;

		constexpr auto scale = Time(period::den) / period::num;
		auto scaled = rep(seconds * scale);
	
		DURATION duration (scaled);
		return duration;
	}
}

