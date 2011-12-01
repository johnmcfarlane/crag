//
//  defs.h
//  crag
//
//  Created by John McFarlane on 7/18/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


// forward-declarations
template <typename S, int D> class Vector;
template <typename S, int R, int C> class Matrix;
template <typename S> class Transformation;
template <typename S, int D> class Ray;


namespace gfx
{
	// Geometric types currently use double and are converted 
	// inside - or close to - the graphics API.
	typedef double Scalar;
	typedef Vector<Scalar, 3> Vector;
	typedef Matrix<Scalar, 3, 3> Matrix33;
	typedef Matrix<Scalar, 4, 4> Matrix44;
	typedef Transformation<Scalar> Transformation;
	typedef Ray<Scalar, 3> Ray;
	
	// unique identifier for gfx::Object;
	// used when refering to objects from outside the render thread
	typedef smp::Uid Uid;
	
	// base class of objects which are rendered
	class Object;
	
	// storage type for Object
	typedef std::map<Uid, Object *> ObjectMap;
}
