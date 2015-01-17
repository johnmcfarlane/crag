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

#include "core/config.h"

////////////////////////////////////////////////////////////////////////////////
// CRAG_COMPILER_MSVC specifics

#if defined(CRAG_COMPILER_MSVC)

#pragma warning(disable: 4200)

// warning C4996: 'strcpy': This function or variable may be unsafe. Consider using strcpy_s instead...
#pragma warning(disable : 4996)	// ... except that strcpy_s is not yet a standard.

#define SIZE_T_FORMAT_SPEC "%Iu"

// happens to be defined in ODE with a very similar value;
// causes a problem in VC++ project because of the way libs are built
#define HAVE_M_PI

// required by VS2013 and older
#define WIN32_C2144_WORKAROUND	// syntax error : 'char' should be preceded by ';' (threadlocal)
#define WIN32_C2327_WORKAROUND	// not a type name, static, or enumerator (intrusive list)
#define WIN32_C3646_WORKAROUND	// 'noexcept' : unknown override specifier
#define WIN32_C3861_WORKAROUND	// 'alignof' : identifier not found (__alignof and _snprintf)

// required by all versions
#define WIN32_C2079_WORKAROUND	// undefined struct caused by missing include directive
#define WIN32_C2338_WORKAROUND	// caused by differences in pointer sizes between VC & other compilers

#else

#define SIZE_T_FORMAT_SPEC "%zu"

#endif


////////////////////////////////////////////////////////////////////////////////
// CRAG_OS_X specifics

#if defined (CRAG_OS_X)
#define _XOPEN_SOURCE
#define _DARWIN_C_SOURCE
#endif


//////////////////////////////////////////////////////////////////////
// SDL include

#include <SDL.h>

//////////////////////////////////////////////////////////////////////
// OpenGL includes

#define GL_GLEXT_PROTOTYPES 1

// TODO: Re-evaluate "SDL_opengl.h"
#if defined(GRAG_GLES)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#if defined(GRAG_GL)
#if defined(CRAG_OS_X)
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/glu.h>
#else
#if defined(CRAG_OS_WINDOWS)
#define GLEW_STATIC
#include <GL/glew.h>	 // must be included before gl.h 
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif


//////////////////////////////////////////////////////////////////////
// standard includes.

// C standard library
#include <cassert>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>

// C++ standard library

#if defined(CRAG_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable: 4530)
#endif

#include <algorithm>
#include <array>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#if defined(CRAG_COMPILER_MSVC)
#pragma warning(pop)
#endif

//////////////////////////////////////////////////////////////////////
// core includes.

#include "core/core.h"
#include "core/debug.h"
#include "core/floatOps.h"
#include "core/intOps.h"
#include "core/verify.h"
#include "core/intrusive_list.h"

#include "geom/Vector2.h"
#include "geom/Vector3.h"
#include "geom/Vector4.h"
#include "geom/Distance.h"
#include "geom/Magnitude.h"
#include "geom/Plane.h"
#include "geom/Ray.h"
#include "geom/Sphere.h"
#include "geom/Triangle.h"
