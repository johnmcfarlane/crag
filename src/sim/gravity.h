//
//  gravity.h
//  crag
//
//  Created by John McFarlane on 11/6/11.
//  Copyright 2009 - 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Engine.h"

namespace sim
{
	// return the force acting on a particle at given position
	Vector3 GetGravitationalForce(Engine & engine, Vector3 position);

	// simulates gravitational interactions between the given entities
	void ApplyGravity(Engine & engine, core::Time delta);
	
	// given gravitational force as provided by a physics::Body,
	// calculates upward direction or returns Zero
	Vector3 GetUp(Vector3 const & gravitational_force);
}
