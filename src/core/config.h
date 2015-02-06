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
#if _MSC_VER >= 1800	// VS2013 and older
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

#if _MSC_VER < 1900	// VS2013
#define WIN32_C2327_WORKAROUND	// not a type name, static, or enumerator (intrusive list)
#define WIN32_C3646_WORKAROUND	// 'noexcept' : unknown override specifier
#define WIN32_C3861_WORKAROUND	// 'alignof' : identifier not found (__alignof and _snprintf)
#endif

#if _MSC_VER < 1800	// pre VS2013
#define WIN32_C2144_WORKAROUND	// syntax error : 'char' should be preceded by ';' (threadlocal)
#endif

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
#elif defined(WIN32)
#define CRAG_OS_WINDOWS
#else
#error unrecognized platform
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