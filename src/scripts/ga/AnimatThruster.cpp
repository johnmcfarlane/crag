//
//  AnimatThruster.cpp
//  crag
//
//  Created by John McFarlane on 2013-03-20.
//	Copyright 2013 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "AnimatThruster.h"

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "core/Roster.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// AnimatThruster member definitions

DEFINE_POOL_ALLOCATOR(AnimatThruster, 80);

AnimatThruster::AnimatThruster(Entity & entity, Ray3 const & ray)
: Thruster(entity, ray, false)
{
	auto & tick_roster = entity.GetEngine().GetTickRoster();
	tick_roster.AddOrdering(& AnimatThruster::TickThrustFactor, & Thruster::Tick);
	tick_roster.AddCommand(* this, & AnimatThruster::TickThrustFactor);
}

AnimatThruster::~AnimatThruster()
{
	auto & tick_roster = GetEntity().GetEngine().GetTickRoster();
	tick_roster.RemoveCommand(* this, & AnimatThruster::TickThrustFactor);
}

void AnimatThruster::TickThrustFactor()
{
}
