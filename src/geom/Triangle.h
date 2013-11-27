//
//  Triangle.h
//  crag
//
//  Created by John on 2013-09-05.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace geom
{
	// a triangle represented by an array of three corner points
	template <typename S, int N>
	class Triangle
	{
	public:
		typedef S Scalar;
		typedef ::geom::Vector<S, N> Vector;
		typedef ::std::array<Vector, 3> Array;

		Triangle() = default;
		Triangle(Triangle const &) = default;
		explicit Triangle(std::initializer_list<Vector> rhs)
		: points(rhs)
		{
		}
		
		Triangle(Vector const & a, Vector const & b, Vector const & c)
		: points({{a, b, c}})
		{
		}

#if defined(CRAG_VERIFY_ENABLED)
		void Verify() const
		{
			for (auto const & point : * this)
			{
				point.Verify();
			}
		}
#endif
		
		// attributes
		Array points;
	};

	// catches pointless casts and bypasses them
	template <typename S, int N>
	Triangle<S, N> const & Cast(Triangle<S, N> const & rhs)
	{
		return rhs;
	}

	// casts between triangles of different scalar types
	template <typename LHS_S, typename RHS_S, int N>
	typename std::enable_if<! std::is_same<LHS_S, RHS_S>::value, Triangle<LHS_S, N>>::type Cast(Triangle<RHS_S, N> const & rhs)
	{
		return Triangle<LHS_S, N>(Cast<LHS_S>(rhs.points[0]), Cast<LHS_S>(rhs.points[1]), Cast<LHS_S>(rhs.points[2]));
	}
	
	template <typename S, int N>
	Vector<S, N> Center(Triangle<S, N> const & t)
	{
		return (t.points[0] + t.points[1] + t.points[2]) / S(3);
	}

	template <typename S, int N>
	Vector<S, N> Normal(Triangle<S, N> const & t)
	{
		return CrossProduct(t.points[1] - t.points[0], t.points[1] - t.points[2]);
	}

	// streaming
	template <typename S, int N>
	std::ostream & operator << (std::ostream & out, Triangle<S, N> const & triangle)
	{
		return out << triangle.points[0] << ";" << triangle.points[1] << ";" << triangle.points[2];
	}

	template <typename S, int N>
	std::istream & operator >> (std::istream & in, Triangle<S, N> const & triangle)
	{
		return in >> triangle.points[0] >> ";" >> triangle.points[1] >> ";" >> triangle.points[2];
	}
}
