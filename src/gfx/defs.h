//
//  defs.h
//  crag
//
//  Created by John McFarlane on 7/18/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace geom
{
	// forward-declarations
	template <typename S, int D> class Vector;
	template <typename S, int R, int C> class Matrix;
	template <typename S> class Transformation;
	template <typename S, int D> class Ray;
}

namespace gfx
{
	namespace ProgramIndex
	{
		enum type
		{
			poly,
			sphere,
			fog,
			disk,
			skybox,
			fixed,
			dont_care,
			max_shader = fixed,
			max_index = dont_care
		};
	}
	
	// geometric types
	typedef float Scalar;
	typedef geom::Vector<Scalar, 3> Vector3;
	typedef geom::Vector<Scalar, 4> Vector4;
	typedef geom::Vector<Scalar, 2> RenderRange;	// near/far z
	typedef geom::Matrix<Scalar, 3, 3> Matrix33;
	typedef geom::Matrix<Scalar, 4, 4> Matrix44;
	typedef geom::Transformation<Scalar> Transformation;
	typedef geom::Ray<Scalar, 3> Ray3;
	
	// unique identifier for gfx::Object;
	// used when refering to objects from outside the render thread
	typedef ipc::Uid Uid;
	
	// base class of objects which are rendered
	class Object;
}
