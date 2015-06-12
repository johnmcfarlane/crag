//
//  sim/VehicleController.cpp
//  crag
//
//  Created by John McFarlane on 11/9/11.
//	Copyright 2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "VehicleController.h"

#include "Thruster.h"

#include "sim/Entity.h"
#include "sim/Engine.h"

#include "core/RosterObjectDefine.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// sim::VehicleController member functions

CRAG_ROSTER_OBJECT_DEFINE(
	VehicleController,
	10,
	Pool::NoCall())

VehicleController::VehicleController(Entity & entity)
: _super(entity)
{
	CRAG_VERIFY(* this);
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(VehicleController, self)
	for (auto & transmitter : self._transmitters)
	{
		CRAG_VERIFY_TRUE(transmitter->GetReceiver());
	}
	for (auto & receiver : self._receivers)
	{
		CRAG_VERIFY(receiver->GetSignal());
	}
CRAG_VERIFY_INVARIANTS_DEFINE_END

void VehicleController::AddTransmitter(TransmitterPtr && transmitter) noexcept
{
	_transmitters.push_back(std::move(transmitter));
}

void VehicleController::AddReceiver(ReceiverPtr && receiver) noexcept
{
	_receivers.push_back(std::move(receiver));
}

VehicleController::TransmitterVector const & VehicleController::GetTransmitters() const noexcept
{
	return _transmitters;
}

VehicleController::ReceiverVector const & VehicleController::GetReceivers() const noexcept
{
	CRAG_VERIFY(* this);

	return _receivers;
}
