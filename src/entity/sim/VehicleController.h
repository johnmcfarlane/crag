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

#include "Thruster.h"

namespace sim
{
	// An entity with its own locomotion.
	class VehicleController : public Controller
	{
		////////////////////////////////////////////////////////////////////////////////
		// types

		typedef Controller _super;
	public:
		typedef std::unique_ptr<Thruster> ThrusterPtr;
		typedef std::vector<ThrusterPtr> ThrusterVector;

		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		VehicleController(Entity & entity);
		~VehicleController();

		CRAG_VERIFY_INVARIANTS_DECLARE(VehicleController);
		
		// Add a Thruster
		void AddThruster(ThrusterPtr && thruster);
	protected:
		void PopThruster();

	private:

		////////////////////////////////////////////////////////////////////////////////
		// data
		ThrusterVector _thrusters;
	};
}
