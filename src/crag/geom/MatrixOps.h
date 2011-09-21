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


// TODO: Rename this file


template<typename S> Matrix4<S> TranslationMatrix(Vector<S, 3> const & translation)
{
	return Matrix4<S>(1, 0, 0, translation.x,
					  0, 1, 0, translation.y, 
					  0, 0, 1, translation.z,
					  0, 0, 0, 1);
}

template<typename S> Vector<S, 3> TranslationVector(Matrix4<S> const & matrix)
{
	return Vector<S, 3>(matrix[0][3], matrix[1][3], matrix[2][3]);
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

template<typename S> Matrix4<S> Transformation(Vector<S, 3> const & translation, Matrix4<S> const & rotation)
{
	// This is equivalent to TranslationMatrix(translation) * rotation
	return Matrix4<S>(rotation[0][0], rotation[0][1], rotation[0][2], translation[0],
					  rotation[1][0], rotation[1][1], rotation[1][2], translation[1],
					  rotation[2][0], rotation[2][1], rotation[2][2], translation[2],
					  0, 0, 0, 1);
}
