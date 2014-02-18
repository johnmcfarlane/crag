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

namespace geom
{
	// an infinite plane represented by a surface point and normal
	template <typename S, int N>
	class Plane
	{
	public:
		typedef S Scalar;
		typedef ::geom::Vector<S, N> Vector;
		typedef ::geom::Triangle<S, N> Triangle;

		Plane() = default;
		Plane(Plane const &) = default;

		Plane(Vector const & _position, Vector const & _normal)
		: position(_position)
		, normal(_normal)
		{
			CRAG_VERIFY(* this);
		}

		Plane(Triangle const & triangle) 
		: Plane(geom::Centroid(triangle), geom::Normal(triangle))
		{
		}

#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(Plane, self)
			CRAG_VERIFY(self.position);
			CRAG_VERIFY(self.normal);
			CRAG_VERIFY_OP(self.normal, !=, Vector::Zero());
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif

		// attributes
		Vector position;	// arbitrary point on surface
		Vector normal;	// normalized normal to surface
	};

	// catches pointless casts and bypasses them
	template <typename S, int N>
	Plane<S, N> const & Cast(Plane<S, N> const & rhs)
	{
		return rhs;
	}

	// casts between planes of different scalar types
	template <typename LHS_S, typename RHS_S, int N>
	Plane<LHS_S, N> Cast(Plane<RHS_S, N> const & rhs)
	{
		return Plane<LHS_S, N>(Cast<LHS_S>(rhs.position), Cast<LHS_S>(rhs.normal));
	}
	
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
