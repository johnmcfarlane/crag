//
//  debug.h
//  crag
//
//  Created by john on 4/22/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


////////////////////////////////////////////////////////////////////////////////
// Statement macros

// Semicolon-friendly compound statement.
// Useful for wrapping up macros to appear like a function call.
#define DO_STATEMENT(STATEMENT) do { STATEMENT } while (false)

// Semicolon-friendly empty statement.
#define DO_NOTHING DO_STATEMENT()


////////////////////////////////////////////////////////////////////////////////
// Verify / Dump flags

#if ! defined (NDEBUG)
#define VERIFY
#elif defined(VERIFY)
#error VERIFY defined but NDEBUG is not
#endif


////////////////////////////////////////////////////////////////////////////////
// Anonymous namespace wrapper

#if defined (NDEBUG)
#define ANONYMOUS_BEGIN namespace {
#define ANONYMOUS_END }
#define ANONYMOUS
#else
#define ANONYMOUS_BEGIN namespace ANON {
#define ANONYMOUS_END }
#define ANONYMOUS ANON
#endif


////////////////////////////////////////////////////////////////////////////////
// Misc debug helpers


// FUNCTION_SIGNATURE - a string containing the signature of the current function
#if defined(__GNUC__)
#define FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#elif defined(WIN32)
#define FUNCTION_SIGNATURE __FUNCSIG__
#else
#define FUNCTION_SIGNATURE __func__
#endif


// TRUNCATE_STRING
#define MESSAGE_TRUNCATE(STRING, TARGET_LENGTH) STRING + std::max(0, (int)sizeof(STRING) - 1 - TARGET_LENGTH)


// MESSAGE - general purpose console output macro
#define MESSAGE(OUT, FORMAT, ...) fprintf(OUT, "%32s:%3d:" FORMAT " [%s]\n", MESSAGE_TRUNCATE(__FILE__, 32), __LINE__, ## __VA_ARGS__, FUNCTION_SIGNATURE)


// DEBUG_MESSAGE - debug build-only stdout output for useful development information
#if defined(NDEBUG)
#define DEBUG_MESSAGE(...) DO_NOTHING
#else
#define DEBUG_MESSAGE(FORMAT, ...) MESSAGE(stdout, FORMAT, ## __VA_ARGS__)
#endif


// ERROR_MESSAGE - output serious error messages to stderr in all builds
#define ERROR_MESSAGE(...) fprintf(stderr, __VA_ARGS__)


// BREAK - interrupt execution
#if defined(WIN32)
#define BREAK() __debugbreak()
#elif defined(__GNUC__)
#if defined(__i386__)
#define BREAK() asm("int3")
#else
#include <signal.h>
#define BREAK() raise(SIGTRAP)
// NB: __builtin_trap() is another possability
#endif
#else
#define BREAK() assert(false)
#endif


// DEBUG_BREAK - debug build-only execution interruption with error message
#if defined(NDEBUG)
#define DEBUG_BREAK(...) DO_NOTHING
#else
#define DEBUG_BREAK(FORMAT, ...) \
	DO_STATEMENT ( \
		DEBUG_MESSAGE(FORMAT, ## __VA_ARGS__); \
		BREAK(); \
	)
#endif


// ASSERT - interrupt execution in a debug build iff given condition fails
#if defined(NDEBUG)
#define ASSERT(CONDITION) DO_NOTHING
#else
#define ASSERT(CONDITION) \
	DO_STATEMENT ( \
		if (! (CONDITION)) { \
			DEBUG_BREAK("ASSERT: '%s'", #CONDITION); \
		} \
	)
#endif


// standard library error reporter
#if defined(NDEBUG)
#define AssertErrno() DO_NOTHING
#else
#define AssertErrno() \
	DO_STATEMENT ( \
		if (errno != 0) { \
			DEBUG_BREAK("errno: %d '%s'", errno, strerror(errno)); \
		} \
	)
#endif


// SDL Error Reporter
#define DEBUG_BREAK_SDL() \
	DO_STATEMENT ( \
		DEBUG_BREAK("SDL error: '%s'", SDL_GetError()); \
	)


//////////////////////////////////////////////////////////////////////
// FUNCTION_NO_REENTRY
// 
// Prevent function from being called twice simultaneously,
// either recursively of in multiple threads.
// Put at the top of a function. 

#if defined(NDEBUG) || defined(WIN32)
#define FUNCTION_NO_REENTRY DO_NOTHING
#else
class ReentryGuard
{
	int & _counter;
public:
	ReentryGuard(int & counter);
	~ReentryGuard();
};
#define FUNCTION_NO_REENTRY \
static int counter = 0; \
ReentryGuard reentry_guard(counter);
#endif


////////////////////////////////////////////////////////////////////////////////
// Verify helper functions

#if defined(VERIFY)

#define VerifyTrue(CONDITION) \
	DO_STATEMENT( \
		if (! (CONDITION)) { \
			DEBUG_BREAK("VerifyTrue(%s)", #CONDITION); \
		} \
	)

#define VerifyEqual(A, B) \
	DO_STATEMENT( \
		if ((A) != (B)) { \
			::std::cerr << #A << '=' << A << "; " #B << '=' << B << std::endl; \
			DEBUG_BREAK("Not equal"); \
		} \
	)

#define VerifyNearlyEqual(A, B, EPSILON) \
	DO_STATEMENT( \
		if (! NearEqual(A, B, EPSILON)) { \
			::std::cerr << #A << '=' << A << "; " \
						<< #B << '=' << B << "; " \
						<< #EPSILON << '=' << EPSILON << std::endl; \
			DEBUG_BREAK("Not nearly equal"); \
		} \
	)

// Verify that the reference is not null - nor a value suspiciously close to null.
template<typename T> void VerifyRef(T const & ref) 
{ 
	ASSERT(& ref >= reinterpret_cast<T *>(0x10000)); 
}

// Additionally, pointers may be null.
template<typename T> void VerifyPtr(T const * ptr) 
{ 
	if (ptr != nullptr) {
		VerifyRef(* ptr); 
	}
}

// Run object's internal verification.
template<typename T>
void VerifyObject(T const & object)
{
	object.Verify();
}

// Verify address of object and run object's internal verification.
template<typename T>
void VerifyObjectRef(T const & ref)
{
	VerifyRef(ref);
	VerifyObject(ref);
}

// Verify pointer to object and (if non-null) run object's internal verification.
template<typename T>
void VerifyObjectPtr(T const * ptr)
{
	if (ptr != nullptr) 
	{
		VerifyObjectRef(* ptr);
	}
}

template<typename T> void VerifyArrayElement(T const * element, T const * begin) 
{ 
	VerifyRef(* begin);								// null check
	VerifyRef(* element);							// null check
	ASSERT(begin <= element);						// in range
	ASSERT(begin + (element - begin) == element);	// alignment
}

template<typename T> void VerifyArrayElement(T const * element, T const * begin, T const * end) 
{ 
	VerifyArrayElement(end, begin);		// valid end pointer
	VerifyArrayElement(element, begin);	// valid element pointer
	ASSERT(element < end);	// element inside top end of range
}

#else

template<typename T> void VerifyRef(T const & ref) { }
template<typename T> void VerifyPtr(T const * ptr) { }
template<typename T> void VerifyObject(T const & obj) { }
template<typename T> void VerifyObjectRef(T const & ref) { }
template<typename T> void VerifyObjectPtr(T const * ptr) { }
template<typename T> void VerifyArrayElement(T const * element, T const * begin) { }
template<typename T> void VerifyArrayElement(T const * element, T const * begin, T const * end) { }

#endif


// pointer to ref
template<typename T> T & ref(T * ptr)
{
	VerifyRef(* ptr);
	return * ptr;
}

// pointer to ref
template<typename T> T const & ref(T const * ptr)
{
	VerifyRef(* ptr);
	return * ptr;
}
