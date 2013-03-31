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
		typedef std::vector<Thruster *> ThrusterVector;
		
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		VehicleController(Entity & entity);
		~VehicleController();

#if defined(VERIFY)
		virtual void Verify() const override;
#endif
		
		// Add a Thruster
		void AddThruster(Thruster * thruster);
	protected:
		void PopThruster();

	private:

		////////////////////////////////////////////////////////////////////////////////
		// data
		ThrusterVector _thrusters;
	};
}
