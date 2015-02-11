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

#include "core/Roster.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// HoverThruster member definitions

DEFINE_DEFAULT_ALLOCATOR(HoverThruster);

HoverThruster::HoverThruster(Entity & entity, Vector3 const & position, Scalar magnitude)
: Thruster(entity, Ray3(position, Vector3::Zero()), false, 1.f)
, _magnitude(magnitude)
{
	ASSERT(_magnitude != 0);
	
	auto & tick_roster = entity.GetEngine().GetTickRoster();
	tick_roster.AddOrdering(& HoverThruster::TickThrustDirection, & Thruster::Tick);
	tick_roster.AddCommand(* this, & HoverThruster::TickThrustDirection);
	
	SetThrustFactor(1.f);
}

HoverThruster::~HoverThruster()
{
	auto & tick_roster = GetEntity().GetEngine().GetTickRoster();
	tick_roster.RemoveCommand(* this, & HoverThruster::TickThrustDirection);
}

void HoverThruster::TickThrustDirection(HoverThruster * thruster)
{
	auto location = thruster->GetEntity().GetLocation();
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
	
	auto up_thrust = up * thruster->_magnitude;
	auto transformation = body.GetTransformation();
	
	auto new_ray = thruster->GetRay();
	new_ray.direction = geom::Inverse(transformation).Rotate(up_thrust);
	thruster->SetRay(new_ray);
}
