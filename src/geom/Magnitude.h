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
#include "Sphere.h"
#include "Triangle.h"

namespace geom
{
	////////////////////////////////////////////////////////////////////////////////
	// Magnitude

	template<typename T> 
	auto Magnitude(T const &o) -> decltype(MagnitudeSq(o))
	{
		auto magnitude_squared = MagnitudeSq(o);
		return std::sqrt(magnitude_squared);
	}

	// Changes v to unit magnitude.
	// Is undefined when input is zero magnitude.
	template <typename T> 
	void Normalize(T & v)
	{
		v *= InvSqrt(MagnitudeSq(v));
	}

	// Returns a copy of v with unit magnitude.
	// Is undefined when input is zero magnitude.
	template <typename T>
	T Normalized(T v)
	{
		Normalize(v);
		return v;
	}

	// Changes v to given magnitude.
	// Is undefined when magnitude is zero.
	template <typename T, typename S> 
	void Resize(T & v, S magnitude)
	{
		v *= (InvSqrt(MagnitudeSq(v)) * magnitude);
	}

	// Returns a copy of v with given magnitude.
	// Is undefined when input is zero.
	template <typename T, typename S> 
	T Resized(T v, S magnitude)
	{
		Resize(v, magnitude);
		return v;
	}

	// Minimizes v to given magnitude.
	template <typename T, typename S> 
	void Clamp(T & v, S magnitude)
	{
		auto magnitude_squared = MagnitudeSq(v);
		if (magnitude_squared > Squared(magnitude))
		{
			v *= (InvSqrt(magnitude_squared) * magnitude);
		}
	}

	// Returns a copy of v no greater than given magnitude.
	template <typename T, typename S> 
	T Clamped(T v, S magnitude)
	{
		Clamp(v, magnitude);
		return v;
	}


	////////////////////////////////////////////////////////////////////////////////
	// Area

	// Given the magnitudes of the sides of a triangle, returns the area of the triangle.
	template<typename S> S TriangleArea(S a, S b, S c)
	{
		S p = (a + b + c) * static_cast<S> (.5);	// half perimeter
		S area = std::sqrt(p * (p - a) * (p - b) * (p - c));
		return area;
	}

	// Given the positions of the corners of a triangle, returns the area of the triangle.
	template<typename S, int N> S Area(Triangle<S, N> const & t)
	{
		S ab = Magnitude(t.points[0] - t.points[1]);
		S bc = Magnitude(t.points[1] - t.points[2]);
		S ca = Magnitude(t.points[2] - t.points[0]);
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

	////////////////////////////////////////////////////////////////////////////////
	// comparison

	// for comparison between triangles
	template<typename S, int N>
	S Magnitude(Triangle<S, N> const & t)
	{
		return Area(t);
	}

	// for comparison between spheres
	template<typename S, int N>
	S Magnitude(Sphere<S, N> const & s)
	{
		return s.radius;
	}

	// Vector (uses mag-squared hack)
	template <typename S, int N>
	bool operator > (Vector<S, N> const & lhs, Vector<S, N> const & rhs)
	{
		return MagnitudeSq(lhs) > MagnitudeSq(rhs);
	}

	template <typename S, int N>
	bool operator >= (Vector<S, N> const & lhs, Vector<S, N> const & rhs)
	{
		return MagnitudeSq(lhs) >= MagnitudeSq(rhs);
	}

	template <typename S, int N>
	bool operator < (Vector<S, N> const & lhs, Vector<S, N> const & rhs)
	{
		return MagnitudeSq(lhs) < MagnitudeSq(rhs);
	}

	template <typename S, int N>
	bool operator <= (Vector<S, N> const & lhs, Vector<S, N> const & rhs)
	{
		return MagnitudeSq(lhs) <= MagnitudeSq(rhs);
	}

	// Ray (uses mag-squared hack)
	template <typename S, int N>
	bool operator > (Ray<S, N> const & lhs, Ray<S, N> const & rhs)
	{
		return MagnitudeSq(lhs) > MagnitudeSq(rhs);
	}

	template <typename S, int N>
	bool operator >= (Ray<S, N> const & lhs, Ray<S, N> const & rhs)
	{
		return MagnitudeSq(lhs) >= MagnitudeSq(rhs);
	}

	template <typename S, int N>
	bool operator < (Ray<S, N> const & lhs, Ray<S, N> const & rhs)
	{
		return MagnitudeSq(lhs) < MagnitudeSq(rhs);
	}

	template <typename S, int N>
	bool operator <= (Ray<S, N> const & lhs, Ray<S, N> const & rhs)
	{
		return MagnitudeSq(lhs) <= MagnitudeSq(rhs);
	}

	// non-Vector, non-Ray (uses magnitude)
	template <typename T>
	bool operator > (T const & lhs, T const & rhs)
	{
		return Magnitude(lhs) > Magnitude(rhs);
	}

	template <typename T>
	bool operator >= (T const & lhs, T const & rhs)
	{
		return Magnitude(lhs) >= Magnitude(rhs);
	}

	template <typename T>
	bool operator < (T const & lhs, T const & rhs)
	{
		return Magnitude(lhs) < Magnitude(rhs);
	}

	template <typename T>
	bool operator <= (T const & lhs, T const & rhs)
	{
		return Magnitude(lhs) <= Magnitude(rhs);
	}
}
