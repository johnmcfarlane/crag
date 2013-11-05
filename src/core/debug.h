//
//  debug.h
//  crag
//
//  Created by john on 4/22/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


// VERIFY macro
#if defined(VERIFY)
#error VERIFY already defined
#endif

//#define CRAG_DEBUG_SHOW_FUNCTION

////////////////////////////////////////////////////////////////////////////////
// Statement macros

// Semicolon-friendly compound statement.
// Useful for wrapping up macros to appear like a function call.
#define DO_STATEMENT(...) do { __VA_ARGS__ } while (false)

// Semicolon-friendly empty statement.
#define DO_NOTHING DO_STATEMENT()


////////////////////////////////////////////////////////////////////////////////
// Debug-only code

#if ! defined(NDEBUG)

#define VERIFY

// Misc debug helpers
namespace core
{
	// thread identifier used in debug output
	void DebugSetThreadName(char const * thread_name);
	char const * DebugGetThreadName();
}

// function parameter wrapper
#define CRAG_DEBUG_PARAM(X) X

// DEBUG_COLOR_ macros
#if defined(WIN32) || defined(__ANDROID__) || defined(__APPLE__)
#define DEBUG_COLOR_FILE
#define DEBUG_COLOR_LINE
#define DEBUG_COLOR_TEXT
#define DEBUG_COLOR_FUNC
#define DEBUG_COLOR_PUNC
#define DEBUG_COLOR_THREAD
#define DEBUG_COLOR_NORM
#else
#define DEBUG_COLOR_FILE "\x1B[32;1m"
#define DEBUG_COLOR_LINE "\x1B[31;1m"
#define DEBUG_COLOR_TEXT "\x1B[37;1m"
#define DEBUG_COLOR_FUNC "\x1B[36;2m"
#define DEBUG_COLOR_PUNC "\x1B[37;2m"
#define DEBUG_COLOR_THREAD "\x1B[31;2m"
#define DEBUG_COLOR_NORM "\x1B[37;22m"
#endif

// TRUNCATE_STRING
#if defined(WIN32)
// VS debugger requires complete file path for navigation features
#define MESSAGE_TRUNCATE(STRING, TARGET_LENGTH) STRING
#else
#define MESSAGE_TRUNCATE(STRING, TARGET_LENGTH) (& STRING[std::max(0, (int)sizeof(STRING) - 1 - TARGET_LENGTH)])
#endif

// FUNCTION_SIGNATURE - a string containing the signature of the current function
#if ! defined(CRAG_DEBUG_SHOW_FUNCTION)
#define FUNCTION_SIGNATURE ""
#elif defined(__GNUC__)
#define FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#elif defined(WIN32)
#define FUNCTION_SIGNATURE __FUNCSIG__
#else
#define FUNCTION_SIGNATURE __func__
#endif

// DEBUG_MESSAGE - debug build-only stdout output for useful development information
#define DEBUG_MESSAGE(FORMAT, ...) PrintMessage(stdout, DEBUG_COLOR_FILE "%32s" DEBUG_COLOR_PUNC "(" DEBUG_COLOR_LINE "%3d" DEBUG_COLOR_PUNC "):" DEBUG_COLOR_TEXT FORMAT DEBUG_COLOR_PUNC " [" DEBUG_COLOR_FUNC "%s" DEBUG_COLOR_PUNC "] " DEBUG_COLOR_THREAD "%s" DEBUG_COLOR_NORM "\n", MESSAGE_TRUNCATE(__FILE__, 32), __LINE__, ## __VA_ARGS__, FUNCTION_SIGNATURE, ::core::DebugGetThreadName())

// BREAK - interrupt execution
#if defined(WIN32)
#define BREAK() __debugbreak()
#elif defined(__ANDROID__)
#include  <android/log.h>
#define BREAK() __android_log_assert("error", "crag", "internal error");
#elif defined(__GNUC__)
#if defined(__i386__)
#define BREAK() asm("int3")
#else
#define BREAK() __builtin_trap()
// NB: raise(SIGTRAP) is an alternative
#endif
#else
#define BREAK() assert(false)
#endif

// DEBUG_BREAK - debug build-only execution interruption with error message
#define DEBUG_BREAK(FORMAT, ...) \
	DO_STATEMENT ( \
		DEBUG_MESSAGE(FORMAT, ## __VA_ARGS__); \
		BREAK(); \
	)

// ASSERT - interrupt execution in a debug build iff given condition fails
#define ASSERT(CONDITION) \
	DO_STATEMENT ( \
		if (! (CONDITION)) { \
			DEBUG_BREAK("ASSERT: '%s'", #CONDITION); \
		} \
	)

// CRAG_DEBUG_DUMP - print out the name/value of an expression
#define CRAG_DEBUG_DUMP(EXPRESSION) DO_STATEMENT( \
		std::ostringstream s; \
		s << #EXPRESSION << " = " << EXPRESSION; \
		DEBUG_MESSAGE("%s", s.str().c_str()); \
	)

// standard library error reporter
#define AssertErrno() \
	DO_STATEMENT ( \
		if (errno != 0) { \
			DEBUG_BREAK("errno: %d '%s'", errno, strerror(errno)); \
		} \
	)

#else

namespace core
{
	inline void DebugSetThreadName(char const *) { }
}

#define CRAG_DEBUG_PARAM(X)
#define DEBUG_MESSAGE(...) DO_NOTHING
#define DEBUG_BREAK(...) UNREACHABLE()
#define ASSERT(CONDITION) DO_NOTHING
#define CRAG_DEBUG_DUMP(EXPRESSION) DO_NOTHING
#define AssertErrno() DO_NOTHING

#endif	// NDEBUG


// Console output
void PrintMessage(FILE * out, char const * format, ...);


// ERROR_MESSAGE - output serious error messages to stderr in all builds
#define ERROR_MESSAGE(FORMAT, ...) PrintMessage(stderr, FORMAT "\n", ## __VA_ARGS__)

// SDL Error Reporter
#define DEBUG_BREAK_SDL() \
	DEBUG_BREAK("SDL error: '%s'", SDL_GetError());	


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

#define VerifyOp(A, OP, B) \
	DO_STATEMENT( \
		const auto & a = (A); \
		const auto & b = (B); \
		if (!(a OP b)) { \
			::std::ostringstream message; \
			message \
				<< '(' << #A "=" << a << ')' \
				<< ' ' << #OP \
				<< " (" << #B "=" << b << ')'; \
			DEBUG_BREAK("Failed: %s", message.str().c_str()); \
		} \
	)

#define VerifyEqual(A, B) \
	VerifyOp(A, ==, B)

#define VerifyNearlyEqual(A, B, EPSILON) \
	DO_STATEMENT( \
		auto a = A; \
		auto b = B; \
		if (! NearEqual(a, b, EPSILON)) { \
			::std::ostringstream message; \
			message << #A "=" << a \
					<< ", " #B "=" << b \
					<< ", " #EPSILON "=" << EPSILON; \
			DEBUG_BREAK("Not nearly equal: %s", message.str().c_str()); \
		} \
	)

#define VerifyNearlyEqualLog(A, B, EPSILON) \
	DO_STATEMENT( \
		auto a = A; \
		auto b = B; \
		if (! NearEqualLog(a, b, EPSILON)) { \
			::std::ostringstream message; \
			message << #A "=" << a \
					<< ", " #B "=" << b \
					<< ", EPSILON=" << EPSILON; \
			DEBUG_BREAK("Not nearly equal: %s", message.str().c_str()); \
		} \
	)

#define VerifyIsUnit(V, EPSILON) VerifyNearlyEqual(geom::Length(V), decltype(geom::Length(V))(1), EPSILON)

// Verify that the reference is not null - nor a value suspiciously close to null.
template<typename T> void VerifyRef(T const & ref) 
{ 
	VerifyOp(& ref, >=, reinterpret_cast<T *>(0x10000)); 
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

inline void VerifyObject(float f)
{
	VerifyEqual(f, f);
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

template<typename T> void VerifyArrayPointer(T const * element, T const * begin) 
{ 
	VerifyRef(* begin);	// null check
	VerifyOp(begin, <=, element);	// in range
	VerifyEqual((reinterpret_cast<char const *>(element) - reinterpret_cast<char const *>(begin)) % sizeof(T), static_cast<std::size_t>(0));	// element alignment
}

// verifies that element is a valid point in range, (begin, end)
template<typename T> void VerifyArrayPointer(T const * element, T const * begin, T const * end) 
{ 
	VerifyArrayPointer(end, begin);		// valid end pointer
	VerifyArrayPointer(element, begin);	// valid element pointer
	VerifyOp(element, <=, end);	// element no further than top end of range
}

// verifies that element is a valid point in range, (begin, end]
template<typename T> void VerifyArrayElement(T const * element, T const * begin, T const * end) 
{ 
	VerifyArrayPointer(end, begin);		// valid end pointer
	VerifyArrayPointer(element, begin);	// valid element pointer
	VerifyOp(element, <, end);	// element inside top end of range
}
#else

#define VerifyTrue(CONDITION) DO_NOTHING
#define VerifyOp(A, OP, B) DO_NOTHING
#define VerifyEqual(A, B) DO_NOTHING
#define VerifyNearlyEqual(A, B, EPSILON) DO_NOTHING
#define VerifyNearlyEqualLog(A, B, EPSILON) DO_NOTHING
#define VerifyIsUnit(V, EPSILON) DO_NOTHING

template<typename T> void VerifyRef(T const &) { }
template<typename T> void VerifyPtr(T const *) { }
template<typename T> void VerifyObject(T const &) { }
template<typename T> void VerifyObjectRef(T const &) { }
template<typename T> void VerifyObjectPtr(T const *) { }
template<typename T> void VerifyArrayPointer(T const *, T const *) { }
template<typename T> void VerifyArrayPointer(T const *, T const *, T const *) { }
template<typename T> void VerifyArrayElement(T const *, T const *, T const *) { }

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
