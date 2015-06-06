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

#include "core/RosterObjectDefine.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// RoverThruster member definitions

CRAG_ROSTER_OBJECT_DEFINE(
	RoverThruster,
	10,
	Pool::CallBefore<& RoverThruster::TickThrustFactor, Thruster, & Thruster::Tick>(Engine::GetTickRoster()))

RoverThruster::RoverThruster(Entity & entity, Ray3 const & ray, ActivationCallback const & activation_callback, bool graphical)
: Thruster(entity, ray, graphical, 0.f)
, _activation_callback(activation_callback)
{
	CRAG_ROSTER_OBJECT_VERIFY(* this);
}

void RoverThruster::TickThrustFactor()
{
	auto activation = _activation_callback();
	SetThrustFactor(activation);
}
