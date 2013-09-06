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
		typedef Vector<S, N> Vector;
		typedef std::array<Vector, 3> Array;

		Triangle() = default;
		Triangle(Triangle const & rhs) = default;
		explicit Triangle(Array const & rhs)
		: points(rhs)
		{
		}
		
		Triangle(Vector const & a, Vector const & b, Vector const & c)
		: Triangle({{a, b, c}})
		{
		}

#if defined(VERIFY)
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
}
