//
//  HoverThruster.cpp
//  crag
//
//  Created by John McFarlane on 2013-03-20.
//	Copyright 2013 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "HoverThruster.h"

#include "sim/Engine.h"
#include "sim/Entity.h"
#include "sim/gravity.h"

#include "physics/Body.h"

#include "core/RosterObjectDefine.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// HoverThruster member definitions

CRAG_ROSTER_OBJECT_DEFINE(
	HoverThruster,
	10,
	Pool::CallBefore<& HoverThruster::TickThrustDirection, Thruster, & Thruster::Tick>(Engine::GetTickRoster()))

HoverThruster::HoverThruster(Entity & entity, Vector3 const & position, Scalar magnitude)
: Thruster(entity, Ray3(position, Vector3::Zero()), false, 1.f)
, _magnitude(magnitude)
{
	ASSERT(_magnitude != 0);

	SetThrustFactor(1.f);
}

void HoverThruster::TickThrustDirection()
{
	auto location = GetEntity().GetLocation();
	if (! location)
	{
		DEBUG_BREAK("missing location");
		return;
	}
	
	auto & body = core::StaticCast<physics::Body>(* location);
	Vector3 up = GetUp(body.GetGravitationalForce());
	if (up == Vector3::Zero())
	{
		return;
	}
	
	auto up_thrust = up * _magnitude;
	auto transformation = body.GetTransformation();
	
	auto new_ray = GetRay();
	new_ray.direction = geom::Inverse(transformation).Rotate(up_thrust);
	SetRay(new_ray);
}
