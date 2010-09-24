/*
 *  pch.h
 *  Crag
 *
 *  Created by John on 2/8/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */


//////////////////////////////////////////////////////////////////////
// The PCH should be included once at the top of every .cpp file.

#if defined(CRAG_PCH) && ! defined(__APPLE__)
#error pch.h included multiple times
#else

#define CRAG_PCH


// Currently, the entire project is C++.
#if defined(__cplusplus)


//////////////////////////////////////////////////////////////////////
// Clean inclusion of <windows.h>.

#if defined(WIN32)

// Set minimum Windows version
#define _WIN32_WINNT _WIN32_WINNT_WIN2K
#include <SDKDDKVer.h>

// Main include for Windows world.
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#undef Yield
#undef IN
#undef OUT

// Avoid some annoying warnings.
//#pragma warning(disable : 4355)
#pragma warning(disable : 4530)

// warning C4996: 'strcpy': This function or variable may be unsafe. Consider using strcpy_s instead...
#pragma warning(disable : 4996)	// ... except that strcpy_s is not yet a standard.

#endif


//////////////////////////////////////////////////////////////////////
// SDL includes.

#if defined(WIN32) || defined(__APPLE__)
#include <SDL.h>
#include <SDL_image.h>
#else
// No actual PCHs for Linux as of yet so omit in favor of individual includes.
//#include <SDL/SDL.h>
//#include <SDL/SDL_image.h>
#endif


//////////////////////////////////////////////////////////////////////
// standard includes.

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstddef>

#include <algorithm>
#include <limits>
#include <iostream>
#include <vector>


//////////////////////////////////////////////////////////////////////
// core includes.

#include "core/core.h"
#include "core/debug.h"
#include "core/floatOps.h"

#include "sys/memory.h"

#include "geom/Vector2.h"
#include "geom/Vector3.h"
#include "geom/Vector4.h"
#include "geom/VectorOps.h"


//////////////////////////////////////////////////////////////////////
// ODE

// ODE is built with either double or single precision.
// Because of the large space which this program simulates,
// double precision is recommended
// which means double-precision ODE
// which means defining dDOUBLE.

#if ! defined(dSINGLE) && ! defined(dDOUBLE)
#define dDOUBLE
#endif


//////////////////////////////////////////////////////////////////////

#endif // defined(__cplusplus)

#endif	// CRAG_PCH
