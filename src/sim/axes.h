/*
 *  Space.h
 *  Crag
 *
 *  Created by John on 1/24/10.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "geom/Transformation.h"
#include "geom/Ray.h"


namespace axes 
{
	enum Axis
	{
		RIGHT,
		FORWARD,
		UP,
		NUM_AXES
	};
	
	// Returns the given axis from the given matrix.
	template<typename S> inline Vector<S, 3> const & GetAxis(Matrix<S, 3, 3> const & rotation, Axis axis)
	{
		return reinterpret_cast<Vector<S, 3> const &> (rotation.GetRow(axis));
	}
	
	// Converts position/matrix combo to a Ray.
	template<typename S> 
	Ray<S, 3> GetCameraRay(Transformation<S> const & transformation)
	{
		return Ray<S, 3>(transformation.GetTranslation(), GetAxis(transformation.GetRotation(), FORWARD));
	}
	
	// converts forward and up vectors into rotation matrix
	template<typename S>
	Matrix<S, 3, 3> Rotation(Vector<S, 3> const & forward, Vector<S, 3> const & up = Vector<S, 3>(0, 0, 1))
	{
		Vector<S, 3> right = Normalized(CrossProduct(forward, up));
		Vector<S, 3> matrix_up = CrossProduct(right, forward);
		
		return Matrix<S, 3, 3>(right.x, right.y, right.z,
							   forward.x, forward.y, forward.z,
							   matrix_up.x, matrix_up.y, matrix_up.z);
	}
}
