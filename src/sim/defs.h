//
//  sim/defs.h
//  crag
//
//  Created by John on 2/11/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "ipc/Handle.h"

namespace geom
{
	// forward-declarations
	template <typename S, int D> class Vector;
	template <typename S, int R, int C> class Matrix;
	template <typename S, int D> class Sphere;
	template <typename S> class Transformation;
	template <typename S, int D> class Ray;
}

namespace ipc
{
	template <typename Engine>
	class Daemon;
}

namespace sim
{
	class Engine;
	DECLARE_CLASS_HANDLE(Entity);	// sim::EntityHandle
	
	// daemon type
	typedef ipc::Daemon<Engine> Daemon;

	// precision used by simulation
	typedef float Scalar;
	
	typedef geom::Vector<Scalar, 2> Vector2;
	typedef geom::Vector<Scalar, 3> Vector3;
	typedef geom::Vector<Scalar, 4> Vector4;
	typedef geom::Ray<Scalar, 3> Ray3;
	typedef geom::Sphere<Scalar, 3> Sphere3;
	typedef geom::Matrix<Scalar, 3, 3> Matrix33;
	typedef geom::Matrix<Scalar, 4, 4> Matrix44;
	typedef geom::Transformation<Scalar> Transformation;
	
	typedef ipc::Uid Uid;
}
