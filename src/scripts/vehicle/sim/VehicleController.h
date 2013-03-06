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
		typedef std::vector<ThrusterPtr> ThrusterVector;
		
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		VehicleController(Entity & entity);
		
		// Add a Thruster
		ThrusterPtr AddThruster(Ray3 const & ray);
		
	private:
		// Called whenever the simulation ticks.
		virtual void Tick() override;

		// provides an opportunity to update the thrust factor
		virtual void TickThrusters() = 0;

		////////////////////////////////////////////////////////////////////////////////
		// data
		ThrusterVector _thrusters;
	};
}
