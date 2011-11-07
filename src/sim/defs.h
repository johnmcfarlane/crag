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
#include "geom/Ray.h"
#include "geom/Sphere.h"
#include "geom/Transformation.h"


namespace sim
{
	// dSINGLE and dDOUBLE are mutually excludive macros required by ODE.
#if defined(dSINGLE)
	typedef float Scalar;
#endif

#if defined(dDOUBLE)
	typedef double Scalar;
#endif
	
	// mirrors sys::Time
	typedef double Time;
	
	typedef ::Vector<Scalar, 2> Vector2;
	typedef ::Vector<Scalar, 3> Vector3;
	typedef ::Vector<Scalar, 4> Vector4;
	typedef ::Ray<Scalar, 3> Ray3;
	typedef ::Sphere<Scalar, 3> Sphere3;
	typedef ::Matrix<Scalar, 3, 3> Matrix33;
	typedef ::Matrix<Scalar, 4, 4> Matrix44;
	typedef ::Transformation<Scalar> Transformation;

	// entity
	class Entity;
	typedef std::vector<Entity *> EntityVector;
}
