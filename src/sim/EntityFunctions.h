//
//  EntityFunctions.h
//  crag
//
//  Created by John McFarlane on 2011/11/15.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"


namespace physics
{
	class Engine;
}


namespace sim
{
	////////////////////////////////////////////////////////////////////////////////
	// forward-declarations
	class EntitySet;
	class Simulation;
	
	
	////////////////////////////////////////////////////////////////////////////////
	// function declarations
	
	// sends render info from simulation to renderer
	void UpdateModels(EntitySet const & entity_set);
	
	// physically attaches the given entities to one another
	void AttachEntities(Uid const & uid1, Uid const & uid2, EntitySet & entity_set, physics::Engine & physics_engine);
}
