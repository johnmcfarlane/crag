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

#include <sim/Engine.h>
#include <sim/Entity.h>

#include <physics/Engine.h>

#include <ipc/Daemon.h>
#include <ipc/Handle_Impl.h>

#include <core/RosterObjectDefine.h>

using namespace physics;

CONFIG_DECLARE(sim_tick_duration, core::Time);

namespace
{
	CONFIG_DEFINE(animat_health_exchange_coefficient, .01f);
	CONFIG_DEFINE(animat_health_loss_per_second, 0.01f);
	//CONFIG_DEFINE(animat_health_loss_per_contact, 0.01f);	// TODO: contact damage
}

////////////////////////////////////////////////////////////////////////////////
// physics::AnimatBody member definitions

CRAG_ROSTER_OBJECT_DEFINE(
	AnimatBody,
	100,
	Pool::CallBase<Body, & Body::PreTick>(Engine::GetPreTickRoster()),
	Pool::Call<& AnimatBody::PostTick>(Engine::GetPostTickRoster()));


CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(AnimatBody, self)
	CRAG_VERIFY(static_cast<SphereBody const &>(self));
	CRAG_VERIFY_OP(self._health, >=, 0);
	CRAG_VERIFY_TRUE(self._health_transmitter.GetReceiver());
CRAG_VERIFY_INVARIANTS_DEFINE_END

AnimatBody::AnimatBody(
	Transformation const & transformation, Vector3 const * velocity, physics::Engine & engine,
	Scalar radius, sim::Transmitter & health_transmitter, sim::Entity & entity) noexcept
: SphereBody(transformation, velocity, engine, radius)
, _health_transmitter(health_transmitter)
, _entity(entity)
{
}

void AnimatBody::PostTick() noexcept
{
	SphereBody::PostTick();

	_health -= animat_health_loss_per_second * sim_tick_duration;
	if (_health <= 0)
	{
		OnZeroHealth();
	}

	_health_transmitter.TransmitSignal(_health);
}

void AnimatBody::OnContact(Body & that_body) noexcept
{
	if (! _health)
	{
		return;
	}

	if (! that_body.HasHealth())
	{
		return;
	}

	auto & that_animat_body = core::StaticCast<AnimatBody>(that_body);
	auto & that_health = that_animat_body._health;
	if (! that_health)
	{
		return;
	}

	// calculate the height advantage of this animat
	auto gravity = geom::Normalized(GetGravitationalForce() + that_body.GetGravitationalForce());
	auto displacement = geom::Normalized(that_body.GetTranslation() - GetTranslation());
	auto height_advantage = geom::DotProduct(gravity, displacement);

	// higher animat wins health from lower animat
	auto health_delta = height_advantage * animat_health_exchange_coefficient;

	auto steal_health = [] (AnimatBody & stealer, AnimatBody & victim, float amount)
	{
		// if all of the victim's health is taken
		if (victim._health <= amount)
		{
			// stealer's reward is capped at total taken
			stealer._health += victim._health;
			victim._health = 0;
			victim.OnZeroHealth();
		}
		else
		{
			stealer._health += amount;
			victim._health -= amount;
		}
	};

	if (health_delta < 0)
	{
		steal_health(that_animat_body, * this, - health_delta);
	}
	else if (health_delta > 0)
	{
		steal_health(* this, that_animat_body, health_delta);
	}
}

bool AnimatBody::HasHealth() const noexcept
{
	return true;
}

void AnimatBody::OnZeroHealth() const noexcept
{
	// (delayed) destruction of the victim
	auto victim_handle = _entity.GetHandle();
	victim_handle.Release();
}