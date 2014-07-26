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

#include "geom/Space.h"

namespace physics
{
	class Engine;
}


namespace sim
{
	////////////////////////////////////////////////////////////////////////////////
	// forward-declarations
	class Engine;
	class Entity;
	
	////////////////////////////////////////////////////////////////////////////////
	// function declarations
	
	// physically attaches the given entities to one another
	void AttachEntities(Entity & entity1, Entity & entity2, physics::Engine & physics_engine);
	
	void ResetSpace(Entity & entity, geom::rel::Vector3 const & delta);
}
