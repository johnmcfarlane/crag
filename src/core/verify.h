//
//  verify.h
//  crag
//
//  Created by john on 2013-11-16.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "debug.h"

// CRAG_VERIFY_ENABLED macro definition
#if defined(CRAG_VERIFY_ENABLED)
#error CRAG_VERIFY_ENABLED already defined
#endif

#if defined(CRAG_DEBUG)
// iff defined, enables verification code
#define CRAG_VERIFY_ENABLED
#endif

#if defined(CRAG_VERIFY_ENABLED)
// iff defined, enables verification specifically of object invariants
//#define CRAG_VERIFY_INVARIANTS_ENABLED
#endif

////////////////////////////////////////////////////////////////////////////////
// Verify helper functions
//
// provide macros to use for debug build-only checks of program correctness

#if defined(CRAG_VERIFY_ENABLED)

// verifies that given expression is true
#define CRAG_VERIFY_TRUE(CONDITION) \
	DO_STATEMENT( \
		if (! (CONDITION)) { \
			DEBUG_BREAK("untrue: \"%s\"", #CONDITION); \
		} \
	)

// verifies that given expression is false
#define CRAG_VERIFY_FALSE(CONDITION) \
	DO_STATEMENT( \
		if (CONDITION) { \
			DEBUG_BREAK("true: \"%s\"", #CONDITION); \
		} \
	)

// verifies that result of expressions applied to binary operator is true
#define CRAG_VERIFY_OP(A, OP, B) \
	DO_STATEMENT( \
		const auto & __a = (A); \
		const auto & __b = (B); \
		if (!(__a OP __b)) { \
			::std::ostringstream message; \
			message \
				<< '(' << #A "=" << __a << ')' \
				<< ' ' << #OP \
				<< " (" << #B "=" << __b << ')'; \
			DEBUG_BREAK("Failed: %s", message.str().c_str()); \
		} \
	)

// verifies that given expressions are equal
#define CRAG_VERIFY_EQUAL(A, B) \
	CRAG_VERIFY_OP(A, ==, B)

// verifies that given enum values are equal
#define CRAG_VERIFY_EQUAL_ENUM(A, B) \
	CRAG_VERIFY_EQUAL(static_cast<intmax_t>(A), static_cast<intmax_t>(B))

// verifies that given expressions are nearly equal within a margin
#define CRAG_VERIFY_NEARLY_EQUAL(A, B, EPSILON) \
	DO_STATEMENT( \
		auto __a = A; \
		auto __b = B; \
		if (! NearEqual(__a, __b, EPSILON)) { \
			::std::ostringstream message; \
			message << #A "=" << __a \
					<< ", " #B "=" << __b \
					<< ", " #EPSILON "=" << EPSILON; \
			DEBUG_BREAK("Not nearly equal: %s", message.str().c_str()); \
		} \
	)

// verifies that given expressions are nearly proportional within a margin
#define CRAG_VERIFY_NEARLY_EQUAL_LOG(A, B, EPSILON) \
	DO_STATEMENT( \
		auto __a = A; \
		auto __b = B; \
		if (! NearEqualLog(__a, __b, EPSILON)) { \
			::std::ostringstream message; \
			message << #A "=" << __a \
					<< ", " #B "=" << __b \
					<< ", EPSILON=" << EPSILON; \
			DEBUG_BREAK("Not nearly equal: %s", message.str().c_str()); \
		} \
	)

// verifies that given argument is a valid pointer
#define CRAG_VERIFY_PTR ::crag__core::VerifyPtr

// verifies that given argument is a valid reference
#define CRAG_VERIFY_REF ::crag::core::VerifyRef

// verifies that given expression is a valid pointer in given range
#define CRAG_VERIFY_ARRAY_POINTER ::crag::core::VerifyArrayPointer

// verifies that given element is a valid member of given range
#define CRAG_VERIFY_ARRAY_ELEMENT ::crag::core::VerifyArrayElement

// verifies that given element is a valid pointer in the given range
#define CRAG_VERIFY_UNIT(V, EPSILON) CRAG_VERIFY_NEARLY_EQUAL(geom::Magnitude(V), decltype(geom::Magnitude(V))(1), EPSILON)

// runs custom verification routine on given type
#if defined(CRAG_VERIFY_INVARIANTS_ENABLED)
#define CRAG_VERIFY(VARIABLE) ::crag::core::VerifyInvariants(VARIABLE)
#else
#define CRAG_VERIFY(OBJECT) CRAG_UNUSED(OBJECT)
#endif

// makes it possible (albeit pointless) to say cout << nullptr
inline std::ostream & operator << (std::ostream & out, std::nullptr_t) noexcept
{
	return out << "<null>";
}

namespace crag
{
	namespace core
	{
		template <typename Type>
		using is_integer = typename std::enable_if<std::is_integral<Type>::value, Type>::type;
		
		template <typename Type>
		using is_real = typename std::enable_if<std::is_floating_point<Type>::value, int>::type;
		
		// Verify that pointer is valid
#if defined(CRAG_COMPILER_MSVC)
		template<typename Type>
		void VerifyPtr(Type const *) noexcept
		{
		}
#else
		template<typename Type>
		void VerifyPtr(Type const * ptr) noexcept
		{
			// TODO: Re-enable when std::align is defined
			//CRAG_VERIFY_EQUAL(ptr, std::align(alignof(Type), sizeof(Type), ptr, sizeof(Type)));
			auto alignment = alignof(Type);
			CRAG_VERIFY_EQUAL(reinterpret_cast<std::size_t>(ptr) & (alignment - 1), 0u);
		}
#endif

		// Verify that the reference is not null - nor a value suspiciously close to null.
		template <typename Type>
		void VerifyRef(Type const & ref) noexcept
		{
			// a reference is everything a pointer is, plus it's non-null
			CRAG_VERIFY_TRUE(& ref);

			VerifyPtr(& ref);
		}

		// void pointer
		inline void VerifyInvariants(void const *) noexcept
		{
		}

		// bool
		inline void VerifyInvariants(bool const & b) noexcept
		{
			static_assert(sizeof(bool) == sizeof(char), "bad assumption about type size");

			VerifyRef(b);
			
			// b must be 0 or 1
			auto c = reinterpret_cast<unsigned char const &>(b);
			auto t = static_cast<unsigned char>(true);
			CRAG_VERIFY_OP(c, <= , t);
		}

		// integer
		template <typename Type, is_integer<Type> = 0>
		void VerifyInvariants(Type) noexcept
		{
		}

		// real
		template <typename Type, typename std::enable_if<std::is_floating_point<Type>::value, int>::type = 0>
		void VerifyInvariants(Type real) noexcept
		{
			CRAG_VERIFY(std::isfinite(real));
		}

		// pointer
		template <typename Type>
		void VerifyInvariants(Type const * pointer) noexcept
		{
			VerifyPtr(pointer);
		}

		// classes with VerifyInvariants function
		template <typename Type, typename std::enable_if<std::is_class<Type>::value, int>::type = 0>
		void VerifyInvariants(Type const & object) noexcept
		{
			VerifyRef(object);
			Type::VerifyInvariants(object);
		}

		// std::shared_ptr
		template <typename Type>
		void VerifyInvariants(std::shared_ptr<Type> const & ptr) noexcept
		{
			VerifyPtr(ptr.get());
		}

		// std::shared_ptr
		template <typename Type>
		void VerifyInvariants(std::unique_ptr<Type> const & ptr)
		{
			VerifyPtr(ptr.get());
		}

		// everything else
		template <typename Type, typename ::std::enable_if<! ::std::is_floating_point<Type>::value && ! std::is_class<Type>::value, int>::type = 0>
		void VerifyInvariants(Type const & object) noexcept
		{
			VerifyRef(object);
		}

		// TODO: the VerifyArray* fns need to work with iterators;
		// TODO: even if that means implementing safe printf or somesuch
		template<typename T>
		void VerifyArrayOffset(T const * offset, T const * array) noexcept
		{ 
			VerifyPtr(offset);	// check pointer
			CRAG_VERIFY_OP(array, <=, offset);	// in range
			CRAG_VERIFY_EQUAL((reinterpret_cast<char const *>(offset) - reinterpret_cast<char const *>(array)) % sizeof(T), static_cast<std::size_t>(0));	// element alignment
		}

		// verifies that element is a valid point in range, (begin, end)
		template<typename T>
		void VerifyArrayPointer(T const * element, T const * begin, T const * end) noexcept
		{
			VerifyArrayOffset(element, begin);
			CRAG_VERIFY_OP(element, >=, begin);
			CRAG_VERIFY_OP(element, <=, end);
		}

		// verifies that element is a valid point in range, (begin, end]
		template<typename T>
		void VerifyArrayElement(T const * element, T const * begin, T const * end) noexcept
		{ 
			VerifyArrayOffset(element, begin);
			CRAG_VERIFY_OP(element, >=, begin);
			CRAG_VERIFY_OP(element, <, end);
		}
	}
}

#else

// no-op versions of the above verification macros
#define CRAG_VERIFY_TRUE(CONDITION) CRAG_UNUSED(CONDITION)
#define CRAG_VERIFY_FALSE(CONDITION) CRAG_UNUSED(CONDITION)
#define CRAG_VERIFY_OP(A, OP, B) DO_STATEMENT(CRAG_UNUSED(A); CRAG_UNUSED(B);)
#define CRAG_VERIFY_EQUAL(A, B) DO_STATEMENT(CRAG_UNUSED(A); CRAG_UNUSED(B);)
#define CRAG_VERIFY_EQUAL_ENUM(A, B) DO_STATEMENT(CRAG_UNUSED(A); CRAG_UNUSED(B);)
#define CRAG_VERIFY_NEARLY_EQUAL(A, B, EPSILON) DO_STATEMENT(CRAG_UNUSED(A); CRAG_UNUSED(B);)
#define CRAG_VERIFY_NEARLY_EQUAL_LOG(A, B, EPSILON) DO_STATEMENT(CRAG_UNUSED(A); CRAG_UNUSED(B);)
#define CRAG_VERIFY_PTR(PTR) CRAG_UNUSED(PTR)
#define CRAG_VERIFY_REF(REF) CRAG_UNUSED(REF)
#define CRAG_VERIFY_ARRAY_POINTER(P, B, E) DO_STATEMENT(CRAG_UNUSED(P); CRAG_UNUSED(B); CRAG_UNUSED(E);)
#define CRAG_VERIFY_ARRAY_ELEMENT(P, B, E) DO_STATEMENT(CRAG_UNUSED(P); CRAG_UNUSED(B); CRAG_UNUSED(E);)
#define CRAG_VERIFY_UNIT(V, EPSILON) DO_STATEMENT(CRAG_UNUSED(V); CRAG_UNUSED(EPSILON);)
#define CRAG_VERIFY(OBJECT) CRAG_UNUSED(OBJECT)

#endif

// macros for declaration and definition of class-specific verification routines
#define CRAG_VERIFY_INVARIANTS_DECLARE(CLASS) static void VerifyInvariants(CLASS const &) noexcept
#define CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(CLASS, OBJECT) \
	void CLASS::VerifyInvariants(CLASS const & OBJECT) noexcept { \
		DO_STATEMENT(if (sizeof(OBJECT)) { });
#define CRAG_VERIFY_INVARIANTS_DEFINE_END }

#define CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(CLASS_TEMPLATE, OBJECT) \
	static void VerifyInvariants(CLASS_TEMPLATE const & OBJECT) noexcept \
	{ \
		DO_STATEMENT(if (sizeof(OBJECT)) { });
#define CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END }

//////////////////////////////////////////////////////////////////////
// ref

// pointer to ref
template<typename T>
T & ref(T * ptr) noexcept
{
	CRAG_VERIFY_REF(* ptr);
	return * ptr;
}
