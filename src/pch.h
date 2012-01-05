//
//  pch.h
//  crag
//
//  Created by John on 2/8/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//


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
#pragma warning(disable : 4530)
#pragma warning(disable : 4200)

// warning C4996: 'strcpy': This function or variable may be unsafe. Consider using strcpy_s instead...
#pragma warning(disable : 4996)	// ... except that strcpy_s is not yet a standard.

#endif


//////////////////////////////////////////////////////////////////////
// SDL includes.

// Disable partial compatability with SDL 1.2.
#define SDL_NO_COMPAT

#if defined(WIN32)
#include <SDL.h>
#else
#include <SDL/SDL.h>
#endif


//////////////////////////////////////////////////////////////////////
// standard includes.

// C standard library
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>

// C++ standard library
#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <vector>


//////////////////////////////////////////////////////////////////////
// core includes.

#include "core/core.h"
#include "core/debug.h"
#include "core/floatOps.h"
#include "core/intrusive_list.h"
#include "core/memory.h"

#include "geom/Vector2.h"
#include "geom/Vector3.h"
#include "geom/Vector4.h"
#include "geom/Distance.h"
#include "geom/Magnitude.h"

#include "smp/Uid.h"


//////////////////////////////////////////////////////////////////////
// ODE

// ODE is built with either double or single precision.
// Because of the large space which this program simulates,
// double precision is recommended
// which means double-precision ODE
// which means defining dDOUBLE.

#if defined(dSINGLE)
#error dSINGLE defined; ODE needs to be double-precision.
#endif

#if ! defined(dDOUBLE)
#error dDOUBLE not defined; ODE needs to be double-precision.
#endif


//////////////////////////////////////////////////////////////////////
// Python

#include "Python.h"


//////////////////////////////////////////////////////////////////////

#endif // defined(__cplusplus)

#endif	// CRAG_PCH
