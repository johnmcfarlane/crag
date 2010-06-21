/*
 *  defs.h
 *  crag
 *
 *  Created by John on 6/20/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "core/Vector3.h"
#include "core/Matrix4.h"

#include "ode/ode.h"


namespace physics
{
#if defined(dSINGLE)
	typedef float Scalar;
#elif defined(dDOUBLE)
	typedef double Scalar;
#endif
	
	typedef Vector3<Scalar> Vector3;
	typedef Matrix4<Scalar> Matrix4;
}
