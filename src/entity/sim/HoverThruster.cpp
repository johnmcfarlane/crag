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

#include "physics/Body.h"

#include "core/Roster.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// HoverThruster member definitions

DEFINE_DEFAULT_ALLOCATOR(HoverThruster);

HoverThruster::HoverThruster(Entity & entity, Vector3 const & position, Scalar magnitude)
: Thruster(entity, Ray3(position, Vector3::Zero()), false)
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

void HoverThruster::TickThrustDirection()
{
	auto location = GetEntity().GetLocation();
	if (! location)
	{
		DEBUG_BREAK("missing location");
		return;
	}
	
	auto & body = core::StaticCast<physics::Body>(* location);
	
	auto gravitational_force = body.GetGravitationalForce();
	auto gravity = geom::Length(gravitational_force);
	if (! gravity)
	{
		return;
	}
	
	auto up_thrust = gravitational_force * _magnitude / - gravity;
	auto transformation = body.GetTransformation();
	
	auto new_ray = GetRay();
	new_ray.direction = geom::Inverse(transformation).Rotate(up_thrust);
	SetRay(new_ray);
}
