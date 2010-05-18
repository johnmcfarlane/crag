/*
 *  gl.h
 *  Crag
 *
 *  Created by John on Apr 23, 2010.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


#if defined(__APPLE__)
#include <OpenGL.h>
#include <CGLCurrent.h>
#include <glu.h>
#elif defined(WIN32)
#define GLEW_STATIC
#define GL_GLEXT_PROTOTYPES
//#include <GL/glxew.h>
#include <GL/glew.h>
#include <GL/gl.h>
//#include <GL/glext.h>
//#include <GL/glx.h>
#include <GL/glu.h>
#else
#define GLEW_STATIC
#define GL_GLEXT_PROTOTYPES
//#include <GL/glxew.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
//#include <GL/glx.h>
#include <GL/glu.h>
#endif

