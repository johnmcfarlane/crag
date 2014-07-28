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

	// Changes v to unit length. 
	// Is undefined when input is zero length.
	template <typename T> 
	void Normalize(T & v)
	{
		v *= InvSqrt(LengthSq(v));
	}

	// Returns a copy of v with unit length. 
	// Is undefined when input is zero length.
	template <typename T>
	T Normalized(T v)
	{
		Normalize(v);
		return v;
	}

	// Changes v to given length. 
	// Is undefined when length is zero.
	template <typename T, typename S> 
	void Resize(T & v, S length)
	{
		v *= (InvSqrt(LengthSq(v)) * length);
	}

	// Returns a copy of v with given length. 
	// Is undefined when input is zero.
	template <typename T, typename S> 
	T Resized(T v, S length)
	{
		Resize(v, length);
		return v;
	}

	// Minimizes v to given length. 
	template <typename T, typename S> 
	void Clamp(T & v, S length)
	{
		auto length_squared = LengthSq(v);
		if (length_squared > Squared(length))
		{
			v *= (InvSqrt(length_squared) * length);
		}
	}

	// Returns a copy of v no greater than given length. 
	template <typename T, typename S> 
	T Clamped(T v, S length)
	{
		Clamp(v, length);
		return v;
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

	// given a regular, convex polygon with points of unit distance from center,
	// returns the area
	template <typename S>
	S constexpr GetPolygonArea(int num_sides)
	{
		return std::sin(S(PI / num_sides)) * std::cos(S(PI / num_sides)) * num_sides;
	}

	////////////////////////////////////////////////////////////////////////////////
	// Volume

	template<typename V> auto TetrahedronVolume(V const & a, V const & b, V const & c, V const & d) -> decltype(DotProduct(a, a))
	{
		return std::abs(DotProduct((a - d), CrossProduct((b - d), (c - d)))) / 6;
	}
}
