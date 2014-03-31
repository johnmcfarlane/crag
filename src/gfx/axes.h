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
	enum class Direction
	{
		right = 0,
		up,
		forward,
		size,
		end = size,
		begin = right
	};
	
	// Returns the direction in the given direction from the given matrix.
	template<typename S>
	geom::Vector<S, 3> GetAxis(geom::Matrix<S, 3, 3> const & rotation, Direction direction)
	{
		auto axis = int(direction);

		CRAG_VERIFY_OP(axis, >=, int(Direction::begin));
		CRAG_VERIFY_OP(axis, <, int(Direction::end));
		
		return rotation.GetColumn(axis);
	}
	
	// Returns the direction in the given direction from the given matrix.
	template<typename S>
	void SetAxis(geom::Matrix<S, 3, 3> & rotation, Direction direction, geom::Vector<S, 3> const & vector)
	{
		auto axis = int(direction);

		CRAG_VERIFY_OP(axis, >=, int(Direction::begin));
		CRAG_VERIFY_OP(axis, <, int(Direction::end));
		
		rotation.SetColumn(axis, vector);
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
	geom::Matrix<S, 3, 3> Rotation(geom::Vector<S, 3> const & axis1, geom::Vector<S, 3> axis2, Direction d1 = Direction::forward, Direction d2 = Direction::up)
	{
		typedef geom::Vector<S, 3> Vector;
		
		// generate axis indices
		auto a1 = int(d1);
		auto a2 = int(d2);
		auto a3 = 3 - (a1 + a2);
		
		CRAG_VERIFY_OP(a1, >=, int(Direction::begin));
		CRAG_VERIFY_OP(a1, <, int(Direction::end));
		CRAG_VERIFY_OP(a2, >=, int(Direction::begin));
		CRAG_VERIFY_OP(a2, <, int(Direction::end));
		CRAG_VERIFY_OP(a3, >=, int(Direction::begin));
		CRAG_VERIFY_OP(a3, <, int(Direction::end));
		
		CRAG_VERIFY_OP(a1, !=, a2);
		CRAG_VERIFY_OP(a2, !=, a3);
		CRAG_VERIFY_OP(a3, !=, a1);
		
		// if inputs are flipped, CrossProduct must be flipped also
		auto cp = (TriMod(a2 + 1) == a1)
		? geom::CrossProduct<S>
		: core::Twizzle<Vector, Vector const &, geom::CrossProduct<S>>;
		
		// set axes apart from one another
		Vector axis3 = Normalized(cp(axis2, axis1));
		axis2 = cp(axis1, axis3);

		// verify that axes are units
		CRAG_VERIFY_NEARLY_EQUAL(Length(axis1), S(1), S(0.0001));
		CRAG_VERIFY_NEARLY_EQUAL(Length(axis2), S(1), S(0.0001));
		CRAG_VERIFY_NEARLY_EQUAL(Length(axis3), S(1), S(0.0001));
		
		// assign to matrix object
		geom::Matrix<S, 3, 3> rotation;
		rotation.SetColumn(a1, axis1);
		rotation.SetColumn(a2, axis2);
		rotation.SetColumn(a3, axis3);
		
		return rotation;
	}
	
	// given an axis of a given direction, D, produce an arbitrary unit rotation
	template<typename S>
	geom::Matrix<S, 3, 3> Rotation(geom::Vector<S, 3> const & axis, Direction d = Direction::forward)
	{
		typedef geom::Vector<S, 3> Vector;
		
		CRAG_VERIFY_NEARLY_EQUAL(Length(axis), S(1), S(0.0001));

		Vector axis_d2 = Normalized(Perpendicular(axis));
		Vector axis_d1 = CrossProduct(axis, axis_d2);
		CRAG_VERIFY_NEARLY_EQUAL(Length(axis_d1), S(1), S(0.0001));
		
		// verify axes are perpendicular to one another
		CRAG_VERIFY_NEARLY_EQUAL(DotProduct(axis, axis_d1), S(0), S(0.0001));
		CRAG_VERIFY_NEARLY_EQUAL(DotProduct(axis_d1, axis_d2), S(0), S(0.0001));
		CRAG_VERIFY_NEARLY_EQUAL(DotProduct(axis_d2, axis), S(0), S(0.0001));

		auto d0 = int(d);
		auto d1 = TriMod(d0 + 1);
		auto d2 = TriMod(d0 + 2);
		
		geom::Matrix<S, 3, 3> rotation;
		rotation.SetColumn(d0, axis);
		rotation.SetColumn(d1, axis_d1);
		rotation.SetColumn(d2, axis_d2);
		
		return rotation;
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

