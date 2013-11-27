//
//  form/RayCastResult.cpp
//  crag
//
//  Created by John McFarlane on 2013-09-20
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "RayCastResult.h"

using namespace form;

////////////////////////////////////////////////////////////////////////////////
// RayCastResult function definitions

Scalar const RayCastResult::_max_distance = std::numeric_limits<Scalar>::max();

RayCastResult::RayCastResult(Vector3 const & normal, Scalar distance, Node const * node)
: _normal(normal)
, _distance(distance)
, _node(node)
{
	CRAG_VERIFY(* this);
}

RayCastResult::operator bool () const
{
	CRAG_VERIFY(* this);
	
	return _distance != _max_distance;
}

Scalar RayCastResult::GetDistance() const
{
	CRAG_VERIFY(* this);
	
	return _distance;
}

Node const * RayCastResult::GetNode() const
{
	return _node;
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(RayCastResult, self)
	if (self._distance != self._max_distance)
	{
		CRAG_VERIFY(self._normal);
		CRAG_VERIFY_NEARLY_EQUAL(geom::Length(self._normal), 1.f, .001f);

		CRAG_VERIFY(self._distance);
		CRAG_VERIFY_OP(self._distance, >=, 0.f);
	}
	else
	{
		CRAG_VERIFY_EQUAL(self._node, static_cast<Node const *>(nullptr));
	}
CRAG_VERIFY_INVARIANTS_DEFINE_END

namespace form
{
	// the sort criteria applied to RayCastResult when choosing the lowest contact
	bool operator<(RayCastResult const & lhs, RayCastResult const & rhs)
	{
		CRAG_VERIFY(lhs);
		CRAG_VERIFY(rhs);

		// Being negative means behind the ray. That's worse than being in front.
		int positive_compare = int(lhs._distance >= 0) - int(rhs._distance >= 0);
		if (positive_compare)
		{
			return positive_compare > 0;
		}

		return lhs._distance < rhs._distance;
	}
}
