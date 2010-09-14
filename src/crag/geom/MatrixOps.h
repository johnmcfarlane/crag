/*
 *  MatrixUtils.h
 *  Crag
 *
 *  Created by John on 12/9/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Matrix4.h"
#include "geom/VectorOps.h"


// 50% chance this is wrong.
template<typename S> Matrix4<S> TranslationMatrix(Vector<S, 3> const & position)
{
	/*return Matrix4<S>(1, 0, 0, 0,
					  0, 1, 0, 0, 
					  0, 0, 1, 0,
					  position.x, position.y, position.z, 1);*/
	return Matrix4<S>(1, 0, 0, position.x,
					  0, 1, 0, position.y, 
					  0, 0, 1, position.z,
					  0, 0, 0, 1);
}

template<typename S> Matrix4<S> DirectionMatrix(Vector<S, 3> const & forward, Vector<S, 3> const & up = Vector<S, 3>(0, 0, 1))
{
	Vector<S, 3> right = Normalized(CrossProduct(forward, up));
	Vector<S, 3> matrix_up = CrossProduct(right, forward);

	return Matrix4<S>(right.x, right.y, right.z, 0,	// T
					forward.x, forward.y, forward.z, 0,	// OD
					matrix_up.x, matrix_up.y, matrix_up.z, 0,	// O
					0, 0, 0, 1);
}
