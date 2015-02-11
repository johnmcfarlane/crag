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

#include "core/Roster.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// RoverThruster member definitions

DEFINE_DEFAULT_ALLOCATOR(RoverThruster);

RoverThruster::RoverThruster(Entity & entity, Ray3 const & ray, ActivationCallback const & activation_callback, bool graphical)
: Thruster(entity, ray, graphical, 0.f)
, _activation_callback(activation_callback)
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

void RoverThruster::TickThrustFactor(RoverThruster * thruster)
{
	auto activation = thruster->_activation_callback();
	thruster->SetThrustFactor(activation);
}
