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
// compiler
//
// specifying the wrong compiler is likely to cause compiler errors

#if defined(__clang__)
#define CRAG_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#define CRAG_COMPILER_GCC
#elif defined(_MSC_VER)
#define CRAG_COMPILER_MSVC
#else
#error unrecognized compiler
#endif

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
#define GRAG_GL
#else
#define GRAG_GLES
#endif

//////////////////////////////////////////////////////////////////////
// primary input method

#if defined(CRAG_MOBILE)
#define CRAG_USE_TOUCH	// build target is predominantly a touch-based device
#endif

#if defined(CRAG_PC) || defined(CRAG_WEB)
#define CRAG_USE_MOUSE	// build target is a keyboard/point device combo
#endif
