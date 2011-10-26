/*
 *  defs.h
 *  crag
 *
 *  Created by John on 6/20/10.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "geom/Vector3.h"
#include "geom/Matrix33.h"
#include "geom/Matrix44.h"

// Notes on installing ODE on OS X:
//	Download the version-numbered release from from source forge.
//	Consult the package's INSTALL.txt file for details of how to build and install ODE.
//	In particular, you must build and install the ODE library using double-precision. 
//	Also, as this project is compiled for i386 with a number of optimizations, more flags may be required.
//	As of writing this, the current ./configure CL that works is as follows:
//		CFLAGS="-arch i386 -fno-exceptions -O3 -funroll-loops -ffast-math -fstrict-aliasing -ftree-vectorize -msse3 -mssse3 -fvisibility=hidden -fvisibility-inlines-hidden" CXXFLAGS="-arch i386 -fno-exceptions -fno-rtti -O3 -funroll-loops -ffast-math -fstrict-aliasing -ftree-vectorize -msse3 -mssse3 -fvisibility=hidden -fvisibility-inlines-hidden" ./configure --enable-double-precision --disable-demos --disable-asserts --prefix=$HOME

// Hack to skip warnings in <ode/misc.h>
#define _ODE_MISC_H_

#include <ode/ode.h>


namespace physics
{
#if defined(dSINGLE)
	typedef float Scalar;
#elif defined(dDOUBLE)
	typedef double Scalar;
#endif
	
	typedef Vector<Scalar, 3> Vector3;
	typedef Matrix<Scalar, 3, 3> Matrix33;
	typedef Matrix<Scalar, 4, 4> Matrix44;
}