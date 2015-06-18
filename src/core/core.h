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
	CLASS(const CLASS&) = delete


//////////////////////////////////////////////////////////////////////
// Missing keywords etc.

#if defined(CRAG_COMPILER_MSVC)
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
#elif defined(CRAG_COMPILER_GCC)
#define thread_local __thread
#endif


//////////////////////////////////////////////////////////////////////
// Optimization directives

// EXPECT - tells compiler to expect EXP==C; returns EXP
#if defined(CRAG_COMPILER_GCC)
#define ASSUME(CONDITION) __builtin_expect(CONDITION, true)
#elif defined(CRAG_COMPILER_MSVC)
#define ASSUME(CONDITION) __assume(CONDITION)
#else
#define ASSUME(CONDITION) (CONDITION)
#endif

// UNREACHABLE - tells the compiler, you can't get here
#if defined(CRAG_COMPILER_GCC) || defined(CRAG_COMPILER_CLANG)
#define UNREACHABLE __builtin_unreachable
#endif

#if defined(CRAG_COMPILER_MSVC)
#define UNREACHABLE() __assume(false)
#endif

namespace crag
{
	namespace core
	{
		//////////////////////////////////////////////////////////////////////
		// element_type - given a container of Ts, yields T

		template <typename CONTAINER>
		struct element_type_s;

		template <typename ELEMENT, int N>
		struct element_type_s <ELEMENT[N]>
		{
			using type = ELEMENT;
		};

		template <typename CONTAINER>
		struct element_type_s
		{
			using type = typename std::remove_const<typename std::remove_reference<decltype(*std::begin(CONTAINER()))>::type>::type;
		};

		template <typename CONTAINER>
		using element_type = typename element_type_s<CONTAINER>::type;

		// element_type tests
		static_assert(std::is_same<char, element_type<char[3]>>::value, "element_type test failed");
		static_assert(std::is_same<double, element_type<std::list<double>>>::value, "element_type test failed");
		static_assert(std::is_same<int, element_type<std::vector<int>>>::value, "element_type test failed");
		static_assert(std::is_same<std::shared_ptr<std::string>, element_type<std::vector<std::shared_ptr<std::string>>>>::value, "element_type test failed");

		//////////////////////////////////////////////////////////////////////
		// hash_combine - http://www.boost.org/doc/libs/1_33_1/doc/html/hash_combine.html

		// returns new seed
		inline constexpr std::size_t hash_combine(std::size_t previous_seed, std::size_t hash_value)
		{
			return previous_seed ^ (hash_value + 0x9e3779b9 + (previous_seed << 6) + (previous_seed >> 2));
		}
	}
}

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

#if defined(CRAG_PC) && defined(CRAG_DEBUG)
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
	template <typename ITERATOR, typename ELEMENT>
	std::size_t get_index(ITERATOR const begin, ELEMENT const & element)
	{
		auto element_ptr = & element;
		assert(element_ptr >= begin);
		
		auto index = element_ptr - begin;
		assert(begin + index == element_ptr);
		
		return index;
	}

	//////////////////////////////////////////////////////////////////////
	// make_transform - simple map/list comprehension riff; makes vector
	// containing result of applying given function to given sequence
	//
	// e.g.:
	//	auto in = std::vector<int>{{ 1, 2, 3 }};
	//	auto f = [](int n){return n*n; };
	//	auto t = make_transform(in, f);
	//	assert((t == std::vector<int>{{ 1, 4, 9 }}));

	template <typename SrcSequence, typename Fn, typename DstElement = typename std::result_of<Fn(crag::core::element_type<SrcSequence>)>::type>
	std::vector<DstElement> make_transform(SrcSequence const & source, Fn unary_function)
	{
		std::vector<DstElement> destination;
		destination.reserve(source.size());
		for (auto const & source_element : source)
		{
			destination.push_back(unary_function(source_element));
		}
		return destination;
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
