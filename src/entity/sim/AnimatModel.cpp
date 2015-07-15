//
//  entity/sim/AnimatModel.cpp
//  crag
//
//  Created by john on 2015-06-29.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "AnimatModel.h"

#include <sim/Engine.h>

#include <gfx/Color.h>
#include <gfx/Engine.h>
#include <gfx/object/Ball.h>

#include <core/RosterObjectDefine.h>

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// AnimatModel member definitions

CRAG_ROSTER_OBJECT_DEFINE(
	AnimatModel,
	100,
	Pool::Call<& AnimatModel::Update>(Engine::GetDrawRoster()));

AnimatModel::AnimatModel(Handle handle, physics::Location const & location) noexcept
: super(handle, location)
{
}

Receiver & AnimatModel::GetHealthReceiver() noexcept
{
	return _health_receiver;
}

void AnimatModel::Update() noexcept
{
	super::Update();

	auto health = _health_receiver.GetSignal();

	GetHandle().Call([health] (gfx::Object & node) {
		using namespace gfx;
		auto & ball = core::StaticCast<Ball>(node);
		auto color = Color4f::Green() * health + Color4f::Red() * (1.f - health);
		ball.SetColor(color);
	});
}
