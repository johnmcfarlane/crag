//
//  Plane.h
//  crag
//
//  Created by John on 2013-09-05.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Triangle.h"
#include "Magnitude.h"

namespace geom
{
	// an infinite plane represented by a surface point and unit normal
	template <typename S, int N>
	class Plane
	{
	public:
		typedef S Scalar;
		typedef ::geom::Vector<S, N> Vector;
		typedef ::geom::Triangle<S, N> Triangle;

		constexpr Plane(Plane const & that) noexcept
			: Plane(that.position, that.normal)
		{ 
		}

		Plane(Vector const & _position, Vector const & _normal)
		: position(_position)
		, normal(_normal)
		{
			CRAG_VERIFY(* this);
		}

		Plane(Triangle const & triangle) 
		: Plane(geom::Centroid(triangle), geom::UnitNormal(triangle))
		{
		}

		Plane & operator=(Plane const & rhs) const noexcept
		{
			CRAG_VERIFY(*this);

			position = rhs.position;
			normal = rhs.normal;

			CRAG_VERIFY(*this);
			return *this;
		}

#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(Plane, self)
			CRAG_VERIFY(self.position);
			CRAG_VERIFY(self.normal);
			CRAG_VERIFY_OP(self.normal, !=, Vector::Zero());
			CRAG_VERIFY_UNIT(self.normal, .0001f);
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif

		// attributes
		Vector const position;	// arbitrary point on surface
		Vector const normal;	// normal to surface
	};

	template <typename S, int N>
	Plane<S, N> MakePlane(Triangle<S, N> const & t)
	{
		return Plane<S, N>(t);
	}

	template <typename S, int N>
	bool operator==(Plane<S, N> const & lhs, Plane<S, N> const & rhs)
	{
		return lhs.position == rhs.position && lhs.normal == rhs.normal;
	}

	template <typename S, int N>
	bool operator!=(Plane<S, N> const & lhs, Plane<S, N> const & rhs)
	{
		return lhs.position != rhs.position || lhs.normal != rhs.normal;
	}

	// streaming
	template <typename S, int N>
	std::ostream & operator << (std::ostream & out, Plane<S, N> const & plane)
	{
		return out << plane.position << "<>" << plane.normal;
	}

	template <typename S, int N>
	std::istream & operator >> (std::istream & in, Plane<S, N> const & plane)
	{
		return in >> plane.position >> "<>" >> plane.normal;
	}
}
