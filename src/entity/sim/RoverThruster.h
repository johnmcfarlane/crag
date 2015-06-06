//
//  RoverThruster.h
//  crag
//
//  Created by John McFarlane on 2013-03-20.
//	Copyright 2013 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Thruster.h"

namespace sim
{
	// thruster for a human-steered vehicle
	class RoverThruster : public Thruster
	{
	public:
		using ActivationCallback = std::function<Scalar ()>;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions

		RoverThruster(Entity & entity, Ray3 const & ray, ActivationCallback const & activation_callback, bool graphical);

		CRAG_ROSTER_OBJECT_DECLARE(RoverThruster);
	private:
		void TickThrustFactor();

		////////////////////////////////////////////////////////////////////////////////
		// variables

		ActivationCallback _activation_callback;
	};
}
