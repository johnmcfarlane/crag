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

	template <typename S, int N>
	Vector<S, N> Centroid(Triangle<S, N> const & t)
	{
		constexpr auto third = S(1) / S(3);
		return (t.points[0] + t.points[1] + t.points[2]) * third;
	}

	template <typename S, int N>
	Vector<S, N> RandomPoint(Triangle<S, N> const & t, S r1, S r2)
	{
		// http://math.stackexchange.com/q/18686
		CRAG_VERIFY_OP(r1, >=, S(0));
		CRAG_VERIFY_OP(r1, <=, S(1));
		CRAG_VERIFY_OP(r2, >=, S(0));
		CRAG_VERIFY_OP(r2, <=, S(1));

		auto root_r1 = std::sqrt(r1);
		return t.points[0] * (S(1) - root_r1)
			+ t.points[1] * (root_r1 * (S(1) - r2))
			+ t.points[2] * (r2 * root_r1);
	}

	template <typename S>
	Vector<S, 3> Normal(Triangle<S, 3> const & t)
	{
		return CrossProduct(t.points[1] - t.points[0], t.points[1] - t.points[2]);
	}

	template <typename S>
	Vector<S, 3> UnitNormal(Triangle<S, 3> const & t)
	{
		return Normalized(Normal(t));
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
