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
#include "core/Vector4.h"
#include "core/VectorOps.h"


// 50% chance this is wrong.
template<typename S> Matrix4<S> TranslationMatrix(Vector3<S> const & position)
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

template<typename S> Matrix4<S> DirectionMatrix(Vector3<S> const & forward, Vector3<S> const & up = Vector3<S>(0, 0, 1))
{
	Vector3<S> right = Normalized(CrossProduct(forward, up));
	Vector3<S> matrix_up = CrossProduct(right, forward);

	return Matrix4<S>(right.x, right.y, right.z, 0,	// T
					forward.x, forward.y, forward.z, 0,	// OD
					matrix_up.x, matrix_up.y, matrix_up.z, 0,	// O
					0, 0, 0, 1);
}
