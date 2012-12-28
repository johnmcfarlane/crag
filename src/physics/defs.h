//
//  defs.h
//  crag
//
//  Created by John on 6/20/10.
//  Copyright 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "geom/Matrix33.h"
#include "geom/Matrix44.h"

namespace physics
{
	typedef dReal Scalar;
	typedef geom::Vector<Scalar, 3> Vector3;
	typedef geom::Ray<Scalar, 3> Ray3;
	typedef geom::Matrix<Scalar, 3, 3> Matrix33;
	typedef geom::Matrix<Scalar, 4, 4> Matrix44;
}
