//
//  entity/physics/AnimatBody.h
//  crag
//
//  Created by John McFarlane on 2015-06-12.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "AnimatBody.h"

#include <entity/sim/Health.h>

#include <sim/Engine.h>

#include <physics/Engine.h>

#include <core/RosterObjectDefine.h>

using namespace physics;

namespace
{
	CONFIG_DEFINE(animat_health_exchange_coefficient, .1f);
	CONFIG_DEFINE(animat_health_gain_per_contact, -.005f);
}

////////////////////////////////////////////////////////////////////////////////
// physics::AnimatBody member definitions

CRAG_ROSTER_OBJECT_DEFINE(
	AnimatBody,
	100,
	Pool::CallBase<Body, & Body::PreTick>(Engine::GetPreTickRoster()),
	Pool::CallBase<Body, & Body::PostTick>(Engine::GetPostTickRoster()));

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(AnimatBody, self)
	CRAG_VERIFY(static_cast<SphereBody const &>(self));
	CRAG_VERIFY_REF(self._health);
CRAG_VERIFY_INVARIANTS_DEFINE_END

AnimatBody::AnimatBody(
	Transformation const & transformation, Vector3 const * velocity, physics::Engine & engine,
	Scalar radius, sim::Health & health) noexcept
: SphereBody(transformation, velocity, engine, radius)
, _health(health)
{
}

void AnimatBody::OnContact(Body & that_body) noexcept
{
	CRAG_VERIFY_OP(& that_body, !=, this);

	if (! that_body.HasHealth())
	{
		_health.IncrementHealth(animat_health_gain_per_contact);
		return;
	}

	if (& that_body < this)
	{
		return;
	}

	auto & that_animat_body = core::StaticCast<AnimatBody>(that_body);

	// calculate the height advantage of this animat
	auto gravity = geom::Normalized(GetGravitationalForce() + that_body.GetGravitationalForce());
	auto displacement = geom::Normalized(that_body.GetTranslation() - GetTranslation());
	auto height_advantage = geom::DotProduct(gravity, displacement);

	// higher animat wins health from lower animat
	auto health_delta = height_advantage * animat_health_exchange_coefficient;

	_health.IncrementHealth(health_delta);
	that_animat_body._health.IncrementHealth(- health_delta);
}

bool AnimatBody::HasHealth() const noexcept
{
	return true;
}
