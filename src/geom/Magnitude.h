//
//  Magnitude.h
//  crag
//
//  Created by John McFarlane on 9/14/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//
//  Functions for determining the magnitude of geometric entities.
//

#pragma once

#include "Ray.h"
#include "Triangle.h"

namespace geom
{
	////////////////////////////////////////////////////////////////////////////////
	// Length

	template<typename T> 
	auto Length(T const & o) -> decltype(LengthSq(o))
	{
		auto length_squared = LengthSq(o);
		return std::sqrt(length_squared);
	}

	// Returns a copy of v with unit length. 
	// Is undefined when input is zero length.
	// Slow but accurate. 
	template <typename T>
	T Normalized(T const & v)
	{
		return v * InvSqrt(LengthSq(v));
	}

	// Returns a copy of v with given length. 
	// Is undefined when input is zero.
	// Slow but accurate. 
	template <typename T, typename S> 
	T Resized(T const & v, S length)
	{
		return v * (InvSqrt(LengthSq(v)) * length);
	}

	// Returns a copy of v with unit length. 
	// Is undefined when input is zero length.
	// Fast but inaccurate. 
	template <typename T>
	T FastNormalized(T const & v)
	{
		return v * FastInvSqrt(LengthSq(v));
	}

	// Converts v to unit vector. 
	// Is undefined when input is zero length.
	// Slow but accurate. 
	template <typename T> 
	void Normalize(T & v)
	{
		v *= InvSqrt(LengthSq(v));
	}

	// Sets v to given length. 
	// Is undefined when length is zero.
	// Slow but accurate. 
	template <typename T, typename S> 
	void Resize(T const & v, S length)
	{
		v *= (InvSqrt(LengthSq(v)) * length);
	}

	// Converts v to unit vector and returns v. 
	// Is undefined when input is zero length.
	// Fast but inaccurate. 
	template <typename T> 
	void FastNormalize(T & v)
	{
		v *= FastInvSqrt(LengthSq(v));
	}


	////////////////////////////////////////////////////////////////////////////////
	// Area

	// Given the lengths of the sides of a triangle, returns the area of the triangle.
	template<typename S> S TriangleArea(S a, S b, S c)
	{
		S p = (a + b + c) * static_cast<S> (.5);	// half perimeter
		S area = std::sqrt(p * (p - a) * (p - b) * (p - c));
		return area;
	}

	// Given the positions of the corners of a triangle, returns the area of the triangle.
	template<typename S, int N> S Area(Triangle<S, N> const & t)
	{
		S ab = Length(t.points[0] - t.points[1]);
		S bc = Length(t.points[1] - t.points[2]);
		S ca = Length(t.points[2] - t.points[0]);
		return TriangleArea(ab, bc, ca);
	}

	// Given the dimensions of a rectangular cuboid, returns the surface area.
	template<typename S> S CuboidArea(Vector<S, 3> const & dimensions)
	{
		return (dimensions.x * dimensions.y + 
				dimensions.y * dimensions.z + 
				dimensions.z * dimensions.x) * S(2);
	}


	////////////////////////////////////////////////////////////////////////////////
	// Volume

	template<typename V> auto TetrahedronVolume(V const & a, V const & b, V const & c, V const & d) -> decltype(DotProduct(a, a))
	{
		return std::abs(DotProduct((a - d), CrossProduct((b - d), (c - d)))) / 6;
	}
}
