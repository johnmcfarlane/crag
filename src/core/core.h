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

#define OBJECT_NO_COPY(CLASS) \
	CLASS & operator=(const CLASS&) = delete; \
	CLASS(const CLASS&) = delete;


//////////////////////////////////////////////////////////////////////
// Missing keywords etc.

#if defined(WIN32)
#define constexpr const
#if defined(WIN32_C3861_WORKAROUND)
#define alignof __alignof
#define snprintf _snprintf
#endif
#if defined(WIN32_C3646_WORKAROUND)
#define noexcept _NOEXCEPT
#endif
#if defined(WIN32_C2144_WORKAROUND)
#define thread_local __declspec(thread)
#endif
#elif defined(__GNUC__)
#define thread_local __thread
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

namespace core
{
	////////////////////////////////////////////////////////////////////////////////
	// StaticCast

	// performs a compile-time cast from type, From, to type, To;
	// in debug builds, performs a run-time test that cast is valid;
	// must have RTTI turned on in debug builds for this to work
	template <typename To, typename From>
	To & StaticCast(From & object)
	{
		To & cast = static_cast<To &>(object);

#if ! defined(__ANDROID__)
		assert(& dynamic_cast<To &>(object) == & cast);
#endif

		return cast;
	}

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

	////////////////////////////////////////////////////////////////////////////////
	// twizzle
	
	// calls given function with parameters reversed
	template <typename O, typename I, O (* f)(I, I)>
	O Twizzle(I lhs, I rhs)
	{
		return f(rhs, lhs);
	}
	
	////////////////////////////////////////////////////////////////////////////////
	// get_index - returns the position of a given element in a given array

	// Given an instance of TYPE which is contained in CLASS as MEMBER,
	// returns the containing object of CLASS.
	template <typename TYPE>
	std::size_t get_index(TYPE const * array, TYPE const & element)
	{
		auto element_ptr = & element;
		assert(element_ptr >= array);
		
		auto index = element_ptr - array;
		assert(array + index == element_ptr);
		
		return index;
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
