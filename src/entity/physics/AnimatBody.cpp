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

#include <geom/utils.h>

#include <core/Random.h>
#include <core/RosterObjectDefine.h>

using namespace physics;

namespace
{
	CONFIG_DEFINE(animat_health_exchange_coefficient, .1f);
	CONFIG_DEFINE(animat_health_gain_per_contact, -.003f);
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
	Vector3 const & position, Vector3 const * velocity, physics::Engine & engine,
	Scalar radius, sim::Health & health) noexcept
: SphereBody(
	Transformation(position, geom::RandomRotation<Scalar>(Random::sequence)),
	velocity,
	engine,
	radius)
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

	// determine down direction
	auto sum_gravitational_force = GetGravitationalForce() + that_body.GetGravitationalForce();
	auto sum_gravitational_force_magnitude = geom::Magnitude(sum_gravitational_force);
	if (! sum_gravitational_force_magnitude)
	{
		// it is unlikely that there can be agreement between the animats upon which direction is down
		return;
	}
	auto down_direction = sum_gravitational_force / sum_gravitational_force_magnitude;
	CRAG_VERIFY_UNIT(down_direction, .0001f);

	// calculate the height advantage of this animat (hack assumes fairly gradual gravity field)
	auto displacement = geom::Normalized(that_body.GetTranslation() - GetTranslation());
	auto height_advantage = geom::DotProduct(down_direction, displacement);

	// higher animat wins health from lower animat
	auto health_delta = height_advantage * animat_health_exchange_coefficient;

	_health.IncrementHealth(health_delta);
	that_animat_body._health.IncrementHealth(- health_delta);
}

bool AnimatBody::HasHealth() const noexcept
{
	return true;
}
