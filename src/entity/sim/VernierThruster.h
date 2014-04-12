//
//  entity/sim/VernierThruster.h
//  crag
//
//  Created by John McFarlane on 2014-03-18.
//	Copyright 2014 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Thruster.h"

namespace physics
{
	class RayCast;
}

namespace sim
{
	// applies force proportional to distance from ground
	struct VernierThruster final : public Thruster
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		VernierThruster(Entity & entity, Ray3 const & ray);

		void Tick() override;

	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables

		std::unique_ptr<physics::RayCast> _ray_cast;	// required to estimate ground effect
	};
}
