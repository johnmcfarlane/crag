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

#include "core/RosterObjectDefine.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// AnimatThruster member definitions

CRAG_ROSTER_OBJECT_DEFINE(
	AnimatThruster,
	100,
	Pool::CallBefore<& AnimatThruster::TickThrustFactor, Thruster, & Thruster::Tick>(Engine::GetTickRoster()))

AnimatThruster::AnimatThruster(Entity & entity, Ray3 const & ray)
: Thruster(entity, ray, false, 1.f)
{
}

void AnimatThruster::TickThrustFactor()
{
}
