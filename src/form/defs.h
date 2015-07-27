//
//  form/defs.h
//  crag
//
//  Created by John on 6/25/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "geom/Ray.h"
#include "geom/Sphere.h"

namespace form
{
	// forward-delcares
	class Node;
	
	// Vector Types
	typedef float Scalar;
	
	typedef geom::Vector<Scalar, 3> Vector3;
	typedef geom::Vector<Scalar, 4> Vector4;
	typedef geom::Plane<Scalar, 3> Plane3;
	typedef geom::Ray<Scalar, 3> Ray3;
	typedef geom::Sphere<Scalar, 3> Sphere3;
	typedef geom::Triangle<Scalar, 3> Triangle3;
	
	// thread-safe node vector
	typedef std::vector<Node *> NodeVector;
}
