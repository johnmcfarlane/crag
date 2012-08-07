//
//  pch.h
//  crag
//
//  Created by John on 2/8/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//
//	The pre-compiled header for the crag project
//

#if defined(__cplusplus)


////////////////////////////////////////////////////////////////////////////////
// WIN32 specifics

#if defined(WIN32)

//////////////////////////////////////////////////////////////////////
// Clean inclusion of <windows.h>.

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


////////////////////////////////////////////////////////////////////////////////
// __APPLE__ specifics

#if defined (__APPLE__)
#define _XOPEN_SOURCE
#endif


//////////////////////////////////////////////////////////////////////
// SDL includes.

// Disable partial compatability with SDL 1.2.
#define SDL_NO_COMPAT

#if defined(WIN32)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif


//////////////////////////////////////////////////////////////////////
// OpenGL includes

// TODO: Re-evaluate "SDL_opengl.h"
#if defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/glu.h>
#else
#define GLEW_STATIC
#define GL_GLEXT_PROTOTYPES
#include <GL/glew.h>	 // must be included before gl.h 
#include <GL/gl.h>
//#include <GL/glext.h>
#include <GL/glu.h>
#endif


//////////////////////////////////////////////////////////////////////
// standard includes.

#include <errno.h>

// C standard library
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>

// C++ standard library
#include <algorithm>
#include <array>
#include <atomic>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

//////////////////////////////////////////////////////////////////////
// core includes.

#include "core/core.h"
#include "core/debug.h"
#include "core/floatOps.h"
#include "core/intOps.h"
#include "core/intrusive_list.h"
#include "core/memory.h"

#include "geom/Vector2.h"
#include "geom/Vector3.h"
#include "geom/Vector4.h"
#include "geom/Distance.h"
#include "geom/Magnitude.h"

#include "smp/Handle_Impl.h"
#include "smp/ObjectBase.h"


//////////////////////////////////////////////////////////////////////
// ODE

// Crag currently requires ODE to be built with double precision
// and for dDOUBLE to be defined.

#if defined(dSINGLE)
#error dSINGLE defined; ODE needs to be double-precision.
#endif

#if ! defined(dDOUBLE)
#error dDOUBLE not defined; ODE needs to be double-precision.
#endif


//////////////////////////////////////////////////////////////////////

#endif // defined(__cplusplus)
