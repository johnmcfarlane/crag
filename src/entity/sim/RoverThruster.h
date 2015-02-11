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

		DECLARE_ALLOCATOR(RoverThruster);

		RoverThruster(Entity & entity, Ray3 const & ray, ActivationCallback const & activation_callback, bool graphical);
		~RoverThruster();
	private:
		static void TickThrustFactor(RoverThruster *);

		////////////////////////////////////////////////////////////////////////////////
		// variables

		ActivationCallback _activation_callback;
	};
}
