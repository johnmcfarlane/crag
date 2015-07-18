//
//  core/config.h
//  crag
//
//  Created by John on 2015-01-16.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//
//	The file that turns compiler/build-received flags and turns them into Crag-specific flags.
//

//////////////////////////////////////////////////////////////////////
// language

#if ! defined(__cplusplus)
#error unrecognized language
#endif

//////////////////////////////////////////////////////////////////////
// build (debug / release)

#if defined(NDEBUG)
#define CRAG_RELEASE
#else
#define CRAG_DEBUG
#endif

//////////////////////////////////////////////////////////////////////
// compiler and compiler 'maturity'
//
// specifying the wrong compiler is likely to cause compiler errors;
// maturity now really just means 'supports variadic lambda capture'

#if defined(__clang__)
#define CRAG_COMPILER_CLANG
#if __clang_major__ > 3 || (__clang_major__ == 3 && __clang_minor__ > 4)
#define CRAG_COMPILER_MATURE
#endif
#elif defined(__GNUG__)
#define CRAG_COMPILER_GCC
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 8)
#define CRAG_COMPILER_MATURE
#endif
#elif defined(_MSC_VER)
#define CRAG_COMPILER_MSVC
#if _MSC_VER >= 1800	// >= VS2013
#define CRAG_COMPILER_MATURE
#endif
#else
#error unrecognized compiler
#endif

//////////////////////////////////////////////////////////////////////
// Compiler differences requiring specific workarounds

#if defined(CRAG_COMPILER_MSVC)

// required by all known versions
#define WIN32_C2079_WORKAROUND	// undefined struct caused by missing include directive
#define WIN32_C2338_WORKAROUND	// caused by differences in pointer sizes between VC & other compilers

#if _MSC_VER < 1900	// < VS2015
#define WIN32_C2327_WORKAROUND	// not a type name, static, or enumerator (intrusive list)
#define WIN32_C3646_WORKAROUND	// 'noexcept' : unknown override specifier
#define WIN32_C3861_WORKAROUND	// 'alignof' : identifier not found (__alignof and _snprintf)
#endif

#if _MSC_VER < 1800	// < VS2013
#define WIN32_C2144_WORKAROUND	// syntax error : 'char' should be preceded by ';' (threadlocal)
#endif

#if _MSC_VER < 1900	// < VS2015
#define constexpr const
#endif

#define CRAG_CONSTEXPR_CTOR

#else	// CRAG_COMPILER_MSVC

#define CRAG_CONSTEXPR_CTOR constexpr

#endif	// CRAG_COMPILER_MSVC

//////////////////////////////////////////////////////////////////////
// OS

#if defined(__ANDROID__)
#define CRAG_OS_ANDROID
#elif defined(__APPLE__)
#define CRAG_OS_X
#elif defined(__linux__)
#define CRAG_OS_LINUX
#elif defined(__pnacl__)
#define CRAG_OS_PNACL
#elif defined(_MSC_VER)
#define CRAG_OS_WINDOWS
#else
#error unrecognized platform
#endif

//////////////////////////////////////////////////////////////////////
// CPU

#if defined(CRAG_COMPILER_MSVC)
# if defined(_M_IX86) || defined(_M_X64)
#  define CRAG_CPU_X86
# endif
#elif defined(CRAG_COMPILER_GCC) || defined(CRAG_COMPILER_CLANG)
# if defined(__i386__) || defined(__amd64__)
#  define CRAG_CPU_X86
# elif defined(__arm__) || defined(__aarch64__)
#  define CRAG_CPU_ARM
# elif defined(__mips__)
#  define CRAG_CPU_MIPS
# elif defined(__ppc__)
#  define CRAG_CPU_PPC
# endif
#endif

#if ! (defined(CRAG_CPU_X86) || defined(CRAG_CPU_ARM) || defined(CRAG_CPU_PPC) || defined(CRAG_CPU_MIPS))
#error unrecognized CPU architecture
#endif

//////////////////////////////////////////////////////////////////////
// categories

#if defined(CRAG_OS_ANDROID)
#define CRAG_MOBILE
#elif defined(CRAG_OS_PNACL)
#define CRAG_WEB
#else
#define CRAG_PC
#endif

//////////////////////////////////////////////////////////////////////
// graphics API

#if defined(CRAG_PC) && ! defined(CRAG_RPI)
#define CRAG_GL
#else
#define CRAG_GLES
#endif

//////////////////////////////////////////////////////////////////////
// primary input method

#if defined(CRAG_MOBILE)
#define CRAG_USE_TOUCH	// build target is predominantly a touch-based device
#endif

#if defined(CRAG_PC) || defined(CRAG_WEB)
#define CRAG_USE_MOUSE	// build target is a keyboard/point device combo
#endif

//////////////////////////////////////////////////////////////////////
// ability to change text color on the console

#if defined(CRAG_OS_LINUX) || defined(CRAG_OS_X)
#define CRAG_ANSI_CONSOLE
#endif
