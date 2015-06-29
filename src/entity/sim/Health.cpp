//
//  entity/sim/Health.h
//  crag
//
//  Created by John McFarlane on 2015-06-29.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include <pch.h>

#include "Health.h"

#include <sim/Engine.h>

#include <core/RosterObjectDefine.h>

using namespace sim;

CONFIG_DECLARE(sim_tick_duration, core::Time);

namespace
{
	CONFIG_DEFINE(animat_health_gain_per_second, -.01f);
}

////////////////////////////////////////////////////////////////////////////////
// sim::Health member definitions

CRAG_ROSTER_OBJECT_DEFINE(
	Health,
	100,
	Pool::Call<& Health::Tick>(Engine::GetTickRoster()));

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Health, self)
	CRAG_VERIFY_REF(self._entity_handle);
	CRAG_VERIFY(self._amount);
	self._transmitter.ForEachReceiver([] (sim::Receiver const & r)
	{
		CRAG_VERIFY(r);
	});
CRAG_VERIFY_INVARIANTS_DEFINE_END

Health::Health(EntityHandle entity_handle)
: _entity_handle(entity_handle)
{
	CRAG_VERIFY(* this);
}

void Health::AddReceiver(sim::Receiver & receiver)
{
	CRAG_VERIFY(* this);

	return _transmitter.AddReceiver(receiver);
}

void Health::Tick() noexcept
{
	CRAG_VERIFY(* this);

	// aging
	IncrementHealth(animat_health_gain_per_second * sim::SignalType(sim_tick_duration));

	// transmit health
	_transmitter.TransmitSignal(_amount);

	// cause death when health drops to zero
	if (_amount <= 0)
	{
		OnDeath();
	}

	CRAG_VERIFY(* this);
}

void Health::IncrementHealth(sim::SignalType delta) noexcept
{
	CRAG_VERIFY(* this);

	_amount += delta;

	CRAG_VERIFY(* this);
}

void Health::OnDeath() noexcept
{
	CRAG_VERIFY(* this);

	_entity_handle.Release();

	CRAG_VERIFY(* this);
}
