//
//  axes.h
//  crag
//
//  Created by John on 1/24/10.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "geom/Transformation.h"
#include "geom/Ray.h"
#include "geom/Sphere.h"

namespace gfx 
{
	enum class Axis
	{
		x,
		y,
		z,
		size
	};
	
	enum class Direction
	{
		right,
		up,
		forward,	// This is how the camera is represented in OpenGL.
		left,
		down,
		backward,
		size,
		negative = left
	};
	
	// Returns the axis in the given direction from the given matrix.
	template<typename S> inline geom::Vector<S, 3> GetAxis(geom::Matrix<S, 3, 3> const & rotation, Direction direction)
	{
		return rotation.GetColumn(static_cast<int>(direction));
	}
	
	// Converts position/matrix combo to a Ray.
	template<typename S> 
	geom::Ray<S, 3> GetCameraRay(geom::Transformation<S> const & transformation)
	{
		return geom::Ray<S, 3>(transformation.GetTranslation(), GetAxis(transformation.GetRotation(), Direction::forward));
	}

	// the depth value pertinent to an OpenGL depth range
	template <typename S>
	S GetDepth(geom::Transformation<S> const & transformation)
	{
		auto & matrix = transformation.GetMatrix();
		return matrix[2][3];
	}
	
	// converts forward and up vectors into rotation matrix
	template<typename S>
	geom::Matrix<S, 3, 3> Rotation(geom::Vector<S, 3> const & forward, geom::Vector<S, 3> const & up)
	{
		VerifyNearlyEqual(Length(forward), S(1), S(0.0001));
		
		geom::Vector<S, 3> right = Normalized(CrossProduct(forward, up));
		geom::Vector<S, 3> matrix_up = CrossProduct(right, forward);
		
		return geom::Matrix<S, 3, 3>(
			right.x, right.y, right.z,
			matrix_up.x, matrix_up.y, matrix_up.z,
			forward.x, forward.y, forward.z);
	}
	
	// converts forward vector into rotation matrix
	template<typename S>
	geom::Matrix<S, 3, 3> Rotation(geom::Vector<S, 3> const & forward)
	{
		VerifyNearlyEqual(Length(forward), S(1), S(0.0001));

		geom::Vector<S, 3> up = Perpendicular(forward);
		Normalize(up);
		
		geom::Vector<S, 3> right = CrossProduct(forward, up);
		VerifyNearlyEqual(Length(right), S(1), S(0.0001));
		
		// verify axes are perpendicular to one another
		VerifyNearlyEqual(DotProduct(forward, right), S(0), S(0.0001));
		VerifyNearlyEqual(DotProduct(right, up), S(0), S(0.0001));
		VerifyNearlyEqual(DotProduct(up, forward), S(0), S(0.0001));

		return geom::Matrix<S, 3, 3>(
			right.x, right.y, right.z,
			up.x, up.y, up.z,
			forward.x, forward.y, forward.z);
	}
	
	// generates rotation of given angle around given axis
	template<typename S>
	geom::Matrix<S, 3, 3> Rotation(Direction direction, S angle)
	{
		auto cos = std::cos(angle);
		auto sin = std::sin(angle);
		
		auto index0 = static_cast<int>(direction);
		auto index1 = TriMod(index0 + 1);
		auto index2 = TriMod(index1 + 1);
		
		geom::Matrix<S, 3, 3> rotation;
		rotation[index0][index0] = 1;
		rotation[index0][index1] = 0;
		rotation[index0][index2] = 0;
		rotation[index1][index0] = 0;
		rotation[index1][index1] = cos;
		rotation[index1][index2] = - sin;
		rotation[index2][index0] = 0;
		rotation[index2][index1] = sin;
		rotation[index2][index2] = cos;
		
		return rotation;
	}
	
	// converts matrix from world space to OpenGL space
	template<typename S>
	geom::Matrix<S, 4, 4> ToOpenGl(geom::Matrix<S, 4, 4> const & matrix)
	{
		return geom::Matrix<S, 4, 4>(matrix[0], matrix[1], - matrix[2], matrix[3]);
	}

	// converts vector from world space to OpenGL space
	template<typename S>
	geom::Vector<S, 3> ToOpenGl(geom::Vector<S, 3> const & vector)
	{
		return geom::Vector<S, 3>(vector[0], vector[1], - vector[2]);
	}

	// converts vector from world space to OpenGL space
	template<typename S>
	geom::Vector<S, 4> ToOpenGl(geom::Vector<S, 4> const & vector)
	{
		return geom::Vector<S, 4>(vector[0], vector[1], - vector[2], vector[3]);
	}
}
