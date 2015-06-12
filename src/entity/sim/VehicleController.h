//
//  sim/VehicleController.h
//  crag
//
//  Created by John McFarlane on 11/9/11.
//	Copyright 2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "sim/Controller.h"

namespace sim
{
	class Transmitter;
	class Receiver;

	// An entity with its own locomotion.
	class VehicleController : public Controller
	{
		////////////////////////////////////////////////////////////////////////////////
		// types

		typedef Controller _super;
	public:
		typedef std::unique_ptr<Transmitter> TransmitterPtr;
		typedef std::vector<TransmitterPtr> TransmitterVector;

		typedef std::unique_ptr<Receiver> ReceiverPtr;
		typedef std::vector<ReceiverPtr> ReceiverVector;

		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		VehicleController(Entity & entity);

		CRAG_ROSTER_OBJECT_DECLARE(VehicleController);
		CRAG_VERIFY_INVARIANTS_DECLARE(VehicleController);

		// Add a Transmitter
		void AddTransmitter(TransmitterPtr && transmitter) noexcept;
		void AddReceiver(ReceiverPtr && receiver) noexcept;

		TransmitterVector const & GetTransmitters() const noexcept;
		ReceiverVector const & GetReceivers() const noexcept;

	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables

		TransmitterVector _transmitters;
		ReceiverVector _receivers;
	};
}
