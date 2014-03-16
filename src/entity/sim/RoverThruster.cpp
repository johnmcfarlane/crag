//
//  RoverThruster.cpp
//  crag
//
//  Created by John McFarlane on 2013-03-20.
//	Copyright 2013 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "RoverThruster.h"

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "core/app.h"
#include "core/Roster.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// RoverThruster member definitions

DEFINE_DEFAULT_ALLOCATOR(RoverThruster);

RoverThruster::RoverThruster(Entity & entity, Ray3 const & ray, SDL_Scancode key)
: Thruster(entity, ray, true)
, _key(key)
{
	auto & tick_roster = entity.GetEngine().GetTickRoster();
	tick_roster.AddOrdering(& RoverThruster::TickThrustFactor, & Thruster::Tick);
	tick_roster.AddCommand(* this, & RoverThruster::TickThrustFactor);
}

RoverThruster::~RoverThruster()
{
	auto & tick_roster = GetEntity().GetEngine().GetTickRoster();
	tick_roster.RemoveCommand(* this, & RoverThruster::TickThrustFactor);
}

void RoverThruster::TickThrustFactor()
{
	bool is_key_down = app::IsKeyDown(_key);
	SetThrustFactor(is_key_down);
}
