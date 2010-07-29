/*
 *  sim/defs.h
 *  Crag
 *
 *  Created by John on 2/11/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */


#pragma once

#if ! defined(CRAG_PCH)
#error pch.h not included.
#endif

#include "geom/Vector2.h"
#include "geom/Vector3.h"
#include "geom/Vector4.h"
#include "geom/Ray3.h"
#include "geom/Sphere3.h"
#include "geom/Matrix4.h"


namespace sim
{
	// dSINGLE and dDOUBLE are mutually excludive macros required by ODE.
#if defined(dSINGLE)
	typedef float Scalar;
#endif

#if defined(dDOUBLE)
	typedef double Scalar;
#endif
	
	typedef ::Vector2<Scalar> Vector2;
	typedef ::Vector3<Scalar> Vector3;
	typedef ::Vector4<Scalar> Vector4;
	typedef ::Ray3<Scalar> Ray3;
	typedef ::Sphere3<Scalar> Sphere3;
	typedef ::Matrix4<Scalar> Matrix4;
}
