//
//  Fiber.h
//  crag
//
//  Created by John McFarlane on 2012-03-28.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

// ensure no macros are defined initially
#if defined(CRAG_USE_FIBER_WIN) || defined(CRAG_USE_FIBER_THREADED) || defined(CRAG_USE_FIBER_POSIX)
#error CRAG_USE_FIBER macro already defined
#endif

// determine which type of fiber support to use
#if defined(CRAG_OS_WINDOWS)
#define CRAG_USE_FIBER_WIN	// use the windows Fiber API
#elif defined(CRAG_PLATFORM_ANDROID) || defined(CRAG_OS_PNACL)
#define CRAG_USE_FIBER_THREADED	// use placeholder lock-step threading
#else
#define CRAG_USE_FIBER_POSIX	// use makecontext etc.
#endif

// include all headers
#include "FiberWin.h"
#include "FiberAndroid.h" 
#include "FiberPosix.h"

