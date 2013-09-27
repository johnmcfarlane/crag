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

Scalar constexpr RayCastResult::_max_distance = std::numeric_limits<Scalar>::max();

RayCastResult::RayCastResult(Vector3 const & normal, Scalar distance, Node const * node)
: _normal(normal)
, _distance(distance)
, _node(node)
{
	VerifyObject(* this);
}

RayCastResult::operator bool () const
{
	VerifyObject(* this);
	
	return _distance != _max_distance;
}

Scalar RayCastResult::GetDistance() const
{
	VerifyObject(* this);
	
	return _distance;
}

Node const * RayCastResult::GetNode() const
{
	return _node;
}

#if defined(VERIFY)
void RayCastResult::Verify() const
{
	if (_distance != _max_distance)
	{
		VerifyObject(_normal);
		VerifyNearlyEqual(geom::Length(_normal), 1.f, .001f);

		VerifyObject(_distance);
		VerifyOp(_distance, >=, 0.f);
	}
	else
	{
		VerifyEqual(_node, static_cast<Node const *>(nullptr));
	}
}
#endif

namespace form
{
	// the sort criteria applied to RayCastResult when choosing the lowest contact
	bool operator<(RayCastResult const & lhs, RayCastResult const & rhs)
	{
		VerifyObjectRef(lhs);
		VerifyObjectRef(rhs);

		// Being negative means behind the ray. That's worse than being in front.
		int positive_compare = int(lhs._distance >= 0) - int(rhs._distance >= 0);
		if (positive_compare)
		{
			return positive_compare > 0;
		}

		return lhs._distance < rhs._distance;
	}
}
