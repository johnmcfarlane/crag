//
//  form/CastRayResult.h
//  crag
//
//  Created by John McFarlane on 2013-09-20
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

namespace form 
{
	////////////////////////////////////////////////////////////////////////////////
	// RayCastResult class definition - represents contact between ray and 
	// polyhedron or a primitive; used by form::CastRay

	class RayCastResult
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types

		// this type dictates the precision used in CastRay
		typedef ::form::Scalar Scalar;

		////////////////////////////////////////////////////////////////////////////////
		// functions

		RayCastResult() = default;
		RayCastResult(Vector3 const & normal, Scalar distance, Node const * node);

		operator bool () const;
		
		Scalar GetDistance() const;

		Node const * GetNode() const;

		friend bool operator<(RayCastResult const & lhs, RayCastResult const & rhs);

#if defined(VERIFY)
		void Verify() const;
#endif
	private:
		////////////////////////////////////////////////////////////////////////////////
		// constants

		static Scalar constexpr _max_distance;

		////////////////////////////////////////////////////////////////////////////////
		// variables

		// the normal of the surface with which the ray contacted
		Vector3 _normal;

		// the distance along the ray (as proportion of ray direction)
		// at which contact occurred
		Scalar _distance = _max_distance;

		// the individual node with which contact occurred
		Node const * _node = nullptr;
	};
}
