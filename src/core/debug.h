//
//  debug.h
//  crag
//
//  Created by john on 4/22/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

//#define CRAG_DEBUG_SHOW_FUNCTION

////////////////////////////////////////////////////////////////////////////////
// Statement macros

// Semicolon-friendly compound statement.
// Useful for wrapping up macros to appear like a function call.
#if defined(CRAG_COMPILER_MSVC)
#define DO_STATEMENT(...) \
	__pragma(warning(push)) \
	__pragma(warning(disable:4127)) \
	do { __VA_ARGS__ } while (false) \
	__pragma(warning(pop))
#else
#define DO_STATEMENT(...) do { __VA_ARGS__ } while (false)
#endif

// Semicolon-friendly empty statement.
#define DO_NOTHING DO_STATEMENT()

// avoids unused variable warnings
#define CRAG_UNUSED(VARIABLE) DO_STATEMENT((void)(VARIABLE);)

// returns true iff it's the first time it's called
#define CRAG_ONCE ([] { \
    static std::atomic<bool> first_time(true); \
    return first_time.exchange(false); } ())

////////////////////////////////////////////////////////////////////////////////
// Debug-only code

#if defined(CRAG_DEBUG)

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
#if defined(CRAG_ANSI_CONSOLE)
#define DEBUG_COLOR_FILE "\x1B[32;1m"
#define DEBUG_COLOR_LINE "\x1B[31;1m"
#define DEBUG_COLOR_TEXT DEBUG_COLOR_NORM
#define DEBUG_COLOR_FUNC "\x1B[36;2m"
#define DEBUG_COLOR_PUNC DEBUG_COLOR_NORM
#define DEBUG_COLOR_THREAD "\x1B[31;2m"
#define DEBUG_COLOR_NORM "\033[m"
#else
#define DEBUG_COLOR_FILE
#define DEBUG_COLOR_LINE
#define DEBUG_COLOR_TEXT
#define DEBUG_COLOR_FUNC
#define DEBUG_COLOR_PUNC
#define DEBUG_COLOR_THREAD
#define DEBUG_COLOR_NORM
#endif

// TRUNCATE_STRING
#if defined(CRAG_OS_WINDOWS)
// VS debugger requires complete file path for navigation features
#define MESSAGE_TRUNCATE(STRING, TARGET_LENGTH) STRING
#else
#define MESSAGE_TRUNCATE(STRING, TARGET_LENGTH) (& STRING[std::max(0, (int)sizeof(STRING) - 1 - TARGET_LENGTH)])
#endif

// FUNCTION_SIGNATURE - a string containing the signature of the current function
#if ! defined(CRAG_DEBUG_SHOW_FUNCTION)
#define FUNCTION_SIGNATURE ""
#elif defined(CRAG_COMPILER_GCC)
#define FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#elif defined(CRAG_COMPILER_MSVC)
#define FUNCTION_SIGNATURE __FUNCSIG__
#else
#define FUNCTION_SIGNATURE __func__
#endif

// DEBUG_MESSAGE - debug build-only stdout output for useful development information
#define DEBUG_MESSAGE(FORMAT, ...) PrintMessage(stdout, DEBUG_COLOR_FILE "%32s" DEBUG_COLOR_PUNC "(" DEBUG_COLOR_LINE "%3d" DEBUG_COLOR_PUNC "):" DEBUG_COLOR_TEXT FORMAT DEBUG_COLOR_PUNC " [" DEBUG_COLOR_FUNC "%s" DEBUG_COLOR_PUNC "] " DEBUG_COLOR_THREAD "%s" DEBUG_COLOR_NORM "\n", MESSAGE_TRUNCATE(__FILE__, 32), __LINE__, ## __VA_ARGS__, FUNCTION_SIGNATURE, ::core::DebugGetThreadName())

// DEBUG_BREAK - debug build-only execution interruption with error message
#define DEBUG_BREAK(FORMAT, ...) \
	DO_STATEMENT ( \
		DEBUG_MESSAGE(FORMAT, ## __VA_ARGS__); \
		CRAG_BREAK(); \
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

#else

namespace core
{
	inline void DebugSetThreadName(char const *) { }
}

#define CRAG_DEBUG_PARAM(X)
#define DEBUG_MESSAGE(...) DO_NOTHING
#define DEBUG_BREAK(...) DO_NOTHING
#define ASSERT(CONDITION) DO_NOTHING
#define CRAG_DEBUG_DUMP(EXPRESSION) DO_NOTHING

#endif

// CRAG_BREAK() - interrupt execution
#if defined(CRAG_COMPILER_MSVC)
#define CRAG_BREAK() __debugbreak()
#elif defined(CRAG_OS_ANDROID)
#include <android/log.h>
#define CRAG_BREAK() __android_log_assert("error", "crag", "internal error")
#elif defined(CRAG_COMPILER_GCC)
#define CRAG_BREAK() __builtin_trap()
#else
#define CRAG_BREAK() assert(false)
#endif

// Console output
void PrintMessage(FILE * out, char const * format, ...);


// ERROR_MESSAGE - output serious error messages to stderr in all builds
#define ERROR_MESSAGE(FORMAT, ...) PrintMessage(stderr, FORMAT "\n", ## __VA_ARGS__)

// SDL Error Reporter
#define CRAG_REPORT_SDL_ERROR() \
	ERROR_MESSAGE("SDL error: \"%s\"", SDL_GetError());


//////////////////////////////////////////////////////////////////////
// FUNCTION_NO_REENTRY
// 
// Prevent function from being called twice simultaneously,
// either recursively of in multiple threads.
// Put at the top of a function. 

#if defined(CRAG_RELEASE) || defined(CRAG_OS_WINDOWS)
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
