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
		////////////////////////////////////////////////////////////////////////////////
		// functions

		DECLARE_ALLOCATOR(RoverThruster);

		RoverThruster(Entity & entity, Ray3 const & ray, SDL_Scancode key);
		~RoverThruster();
	private:
		void TickThrustFactor();

		////////////////////////////////////////////////////////////////////////////////
		// variables

		SDL_Scancode _key;
	};
}
