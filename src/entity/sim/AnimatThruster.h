//
//  AnimatThruster.h
//  crag
//
//  Created by John McFarlane on 2013-03-21.
//	Copyright 2013 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "entity/sim/Thruster.h"

namespace sim
{
	// thruster for an animat-controlled vehicle
	class AnimatThruster : public Thruster
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		AnimatThruster(Entity & entity, Ray3 const & ray);

		CRAG_ROSTER_OBJECT_DECLARE(AnimatThruster);
	private:
		void TickThrustFactor();

		////////////////////////////////////////////////////////////////////////////////
		// variables

	};
}
