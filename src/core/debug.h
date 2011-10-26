/*
 *  debug.h
 *  Crag
 *
 *  Created by john on 4/22/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#if ! defined(CRAG_PCH)
#error pch.h not included.
#endif


////////////////////////////////////////////////////////////////////////////////
// Statement macros

// Semicolon-friendly compound statement,
#define DO_STATEMENT(STATEMENT) do { STATEMENT } while (false)

// Semicolon-friendly empty statement.
#define DO_NOTHING do { } while (false)


////////////////////////////////////////////////////////////////////////////////
// Verify / Dump flags

#if ! defined (NDEBUG)
#define VERIFY
#define DUMP
#elif defined(VERIFY) || defined(DUMP)
#error VERIFY or DUMP defined but NDEBUG is not
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


// Break
#if defined(WIN32)
#define DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__)
#define DEBUG_BREAK() __builtin_trap()
#else
#define DEBUG_BREAK() assert(false)
#endif


#define DEBUG_PRINT_SOURCE __FILE__ << ':' << __LINE__ << ':' << '[' << __FUNCTION__ << ']'


#define DEBUG_BREAK_VERBOSE(MESSAGE) \
	::std::cerr << DEBUG_PRINT_SOURCE << ": break: \"" << MESSAGE << "\"" << std::endl; \
	DEBUG_BREAK()


// Assert
#if defined(NDEBUG)
#define Assert(CONDITION) DO_NOTHING
#else
#define Assert(CONDITION) DO_STATEMENT (if (! (CONDITION)) { DEBUG_BREAK_VERBOSE(#CONDITION); } )
#endif


// SDL Error Reporter
#if defined(NDEBUG)
#define DEBUG_BREAK_SDL() DO_NOTHING
#else
#define DEBUG_BREAK_SDL() DEBUG_BREAK_VERBOSE(SDL_GetError())
#endif


//////////////////////////////////////////////////////////////////////
// FUNCTION_NO_REENTRY
// 
// Prevent function from being called twice simultaneously,
// either recursively of in multiple threads.
// Put at the top of a function. 

#if defined(NDEBUG) || defined(WIN32)
#define FUNCTION_NO_REENTRY DO_NOTHING
#else
#define FUNCTION_NO_REENTRY \
static int counter = 0; \
struct r { r() { assert(++ counter == 1); } ~r() { assert(-- counter == 0); } } R
#endif


////////////////////////////////////////////////////////////////////////////////
// Verify helper functions

#if defined(VERIFY)

#define VerifyTrue(CONDITION) do { if (! (CONDITION)) { DEBUG_BREAK_VERBOSE(#CONDITION); } } while (false)

#define VerifyEqual(A, B, EPSILON) \
	if (! NearEqual(A, B, EPSILON)) { \
		::std::cerr << DEBUG_PRINT_SOURCE << ": break: \"" << #A << " != " << #B << "; " << A << " != " << B << "\"" << std::endl; \
		DEBUG_BREAK(); \
	}


template<typename T> void VerifyRef(T const & ref) 
{ 
#if defined(__APPLE__)
	// 64bit
	//Assert(& ref < reinterpret_cast<T *>(0x120000000));
#elif defined(WIN32)
	// Wish there was a way to get this. 
#else
	// Error in Win32 builds.
	//Assert(& ref < reinterpret_cast<T *>(0x800000000000));
#endif
	Assert(& ref >= reinterpret_cast<T *>(0xd0000)); 
}

// Additionally, pointers may be null.
template<typename T> void VerifyPtr(T const * ptr) 
{ 
	if (ptr != nullptr) {
		VerifyRef(* ptr); 
	}
}

// Run object's internal verification.
#define VerifyObject(OBJECT) (OBJECT).Verify()

// Verify address of object and run object's internal verification.
#define VerifyObjectRef(REF) DO_STATEMENT(VerifyRef(REF); VerifyObject(REF);)

// Verify pointer to object and (if non-null) run object's internal verification.
#define VerifyObjectPtr(PTR) DO_STATEMENT(if (PTR != nullptr) VerifyObjectRef((* PTR));)

template<typename T> void VerifyArrayElement(T const * element, T const * begin) 
{ 
	VerifyRef(* begin);								// null check
	VerifyRef(* element);							// null check
	Assert(begin <= element);						// in range
	Assert(begin + (element - begin) == element);	// alignment
}

template<typename T> void VerifyArrayElement(T const * element, T const * begin, T const * end) 
{ 
	VerifyArrayElement(end, begin);		// valid end pointer
	VerifyArrayElement(element, begin);	// valid element pointer
	Assert(element < end);	// element inside top end of range
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


////////////////////////////////////////////////////////////////////////////////
// Dump helper functions

#if defined(DUMP)

class DumpStream
{
public:
	DumpStream(std::ostream & _out = std::cerr);
	DumpStream(DumpStream & previous);
	
	template <typename F> friend DumpStream & operator << (DumpStream & stream, F const & f)
	{
		stream.out << f;
		return stream;
	}
	
	char const * NewLine() const;
		
private:
	enum { max_indent = 30 };
	
	std::ostream & out;
	char indent [max_indent];
};

#define DUMP_OBJECT(OBJECT, STREAM) DumpStream(STREAM) << OBJECT;
#define DUMP_OPERATOR_DECLARATION(CLASS) ::DumpStream & operator << (::DumpStream & lhs, class CLASS const & rhs)
#define DUMP_OPERATOR_FRIEND_DECLARATION(CLASS) friend ::DumpStream & operator << (::DumpStream & lhs, class CLASS const & rhs)
#define DUMP_OPERATOR_DEFINITION(NAMESPACE, CLASS) ::DumpStream & NAMESPACE::operator << (::DumpStream & lhs, NAMESPACE::CLASS const & rhs)
#define DUMP_OPERATOR_DEFINITION_GLOBAL(CLASS) ::DumpStream & operator << (::DumpStream & lhs, CLASS const & rhs)

#else

#define DUMP_OBJECT(OBJECT, STREAM) DO_NOTHING
#define DUMP_OPERATOR_DECLARATION(CLASS) enum { DUMMY_ENUM }
#define DUMP_OPERATOR_FRIEND_DECLARATION(CLASS) enum { DUMMY_ENUM }

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