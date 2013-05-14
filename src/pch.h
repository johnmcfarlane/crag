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

#undef ERROR
#undef IN
#undef OUT
#undef GetObject
#undef Yield

// Avoid some annoying warnings.
#pragma warning(disable : 4530)
#pragma warning(disable : 4200)

// warning C4996: 'strcpy': This function or variable may be unsafe. Consider using strcpy_s instead...
#pragma warning(disable : 4996)	// ... except that strcpy_s is not yet a standard.

#define SIZE_T_FORMAT_SPEC "%Iu"

// happens to be defined in ODE with a very similar value;
// causes a problem in VC++ project because of the way libs are built
#define HAVE_M_PI

#else

#define SIZE_T_FORMAT_SPEC "%zu"

#endif


////////////////////////////////////////////////////////////////////////////////
// __APPLE__ specifics

#if defined (__APPLE__)
#define _XOPEN_SOURCE
#define _DARWIN_C_SOURCE
#endif


//////////////////////////////////////////////////////////////////////
// ODE includes.

#include <ode/ode.h>


//////////////////////////////////////////////////////////////////////
// SDL includes.

// Disable partial compatability with SDL 1.2.
#define SDL_NO_COMPAT

#if defined(WIN32) || defined(__ANDROID__)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif


//////////////////////////////////////////////////////////////////////
// OpenGL includes

#define GL_GLEXT_PROTOTYPES 1

// TODO: Re-evaluate "SDL_opengl.h"
#if defined(__ANDROID__)

#define CRAG_USE_GLES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#else	// defined(__ANDROID__)

#define CRAG_USE_GL
#if defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/glu.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>	 // must be included before gl.h 
#include <GL/gl.h>
//#include <GL/glext.h>
#include <GL/glu.h>
#endif

#endif	// ! defined(__ANDROID__)


//////////////////////////////////////////////////////////////////////
// standard includes.

#include <errno.h>

// C standard library
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>

// C++ standard library
#include <algorithm>
#include <array>
#include <atomic>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

//////////////////////////////////////////////////////////////////////
// core includes.

#include "core/core.h"
#include "core/debug.h"
#include "core/floatOps.h"
#include "core/intOps.h"
#include "core/intrusive_list.h"
#include "core/object_pool.h"

#include "geom/Vector2.h"
#include "geom/Vector3.h"
#include "geom/Vector4.h"
#include "geom/Distance.h"
#include "geom/Magnitude.h"
#include "geom/Ray.h"
#include "geom/Sphere.h"

#include "ipc/Handle_Impl.h"
#include "ipc/Object.h"


//////////////////////////////////////////////////////////////////////

#endif // defined(__cplusplus)
