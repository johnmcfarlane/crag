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
#if defined(__EXCEPTIONS)
#define CRAG_USE_EXCEPTIONS
#endif
#elif defined(__GNUG__)
#define CRAG_COMPILER_GCC
#if defined(__EXCEPTIONS)
#define CRAG_USE_EXCEPTIONS
#endif
#elif defined(_MSC_VER)
#define CRAG_COMPILER_MSVC
#if defined(_CPPUNWIND)
#define CRAG_USE_EXCEPTIONS
#endif
#else
#error unrecognized compiler
#endif

//////////////////////////////////////////////////////////////////////
// Compiler differences requiring specific workarounds

#if defined(CRAG_COMPILER_MSVC)

// required by all known versions
#define WIN32_C2079_WORKAROUND	// undefined struct caused by missing include directive

#if _MSC_VER < 1900	// < VS2015
#define constexpr const
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
#elif defined(_MSC_VER)
#define CRAG_OS_WINDOWS
#else
#error unrecognized platform
#endif

//////////////////////////////////////////////////////////////////////
// CPU

#if defined(CRAG_COMPILER_MSVC)
#   if defined(_M_AMD64)
#       define CRAG_CPU_X86
#       define CRAG_ARCHITECTURE_WIDTH 64
#   elif defined(_M_IX86)
#       define CRAG_CPU_X86
#       define CRAG_ARCHITECTURE_WIDTH 32
#   elif defined(_M_ARM64)
#       define CRAG_CPU_ARM
#       define CRAG_ARCHITECTURE_WIDTH 64
#   elif defined(_M_ARM)
#       define CRAG_CPU_ARM
#       define CRAG_ARCHITECTURE_WIDTH 32
#   endif
#elif defined(CRAG_COMPILER_GCC) || defined(CRAG_COMPILER_CLANG)
#   if defined(__i386__)
#       define CRAG_CPU_X86
#       define CRAG_ARCHITECTURE_WIDTH 32
#   elif defined(__amd64__)
#       define CRAG_CPU_X86
#       define CRAG_ARCHITECTURE_WIDTH 64
#   elif defined(__arm__)
#       define CRAG_CPU_ARM
#       define CRAG_ARCHITECTURE_WIDTH 32
#   elif defined(__aarch64__)
#       define CRAG_CPU_ARM
#       define CRAG_ARCHITECTURE_WIDTH 64
#   elif defined(__ppc__)
#       define CRAG_CPU_PPC
#       define CRAG_ARCHITECTURE_WIDTH 32
#   elif defined(__ppc64__)
#       define CRAG_CPU_PPC
#       define CRAG_ARCHITECTURE_WIDTH 64
#   endif
#endif

#if ! (defined(CRAG_CPU_X86) || defined(CRAG_CPU_ARM) || defined(CRAG_CPU_PPC) || defined(CRAG_CPU_MIPS))
#error unrecognized CPU architecture
#endif

#if ! defined(CRAG_ARCHITECTURE_WIDTH)
#error unrecognized CPU architecture width
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

#if ! defined(CRAG_GL) && ! defined(CRAG_GLES)
// CRAG_GL or CRAG_GLES must be defined
#error unrecognized graphics API
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
