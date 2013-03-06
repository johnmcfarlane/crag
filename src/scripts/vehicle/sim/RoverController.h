//
//  VehicleController.h
//  crag
//
//  Created by John McFarlane on 11/9/11.
//	Copyright 2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "VehicleController.h"

#include "sim/defs.h"

namespace sim
{
	// controller for a human-steered vehicle
	class RoverController : public VehicleController
	{
		////////////////////////////////////////////////////////////////////////////////
		// types

		typedef std::multimap<SDL_Scancode, ThrusterPtr> InputMap;

	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		DECLARE_ALLOCATOR(RoverController);

		RoverController(Entity & entity);
		
		// Add a Thruster
		void AddThruster(Ray3 const & ray, SDL_Scancode key);
		
	private:
		// applies keyboard input to thrusters
		virtual void TickThrusters() final;

		////////////////////////////////////////////////////////////////////////////////
		// data
		InputMap _inputs;
	};
}
