//
//  EntitySet.h
//  crag
//
//  Created by john on 4/14/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "sim/Entity.h"


namespace physics 
{
	class Body;
}


namespace sim
{
	// forward-declarations
	class Entity;
	class sim::Engine;
	
	
	// Currently deals with simulation time, entity managment and physics ticking.
	class EntitySet
	{
	public:
		// methods
		~EntitySet();

		void Add(Entity & entity);
		void Remove(Entity & entity);
		
		// given a UID, return the corresponding entity
		Entity * GetEntity(smp::Uid uid);
		
		// return all entities
		Entity::List & GetEntities();
		Entity::List const & GetEntities() const;

		void Purge();

	private:
		// variables
		Entity::List entities;
	};
}
