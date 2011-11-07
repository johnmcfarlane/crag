/*
 *  EntitySet.h
 *  Crag
 *
 *  Created by john on 4/14/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "sim/defs.h"

#include "smp/Uid.h"

#include "core/ConfigEntry.h"
#include "core/Singleton.h"


namespace physics 
{
	class Body;
}


namespace sim
{
	// forward-declarations
	class Entity;
	class Simulation;
	
	
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
		EntityVector & GetEntities();
		EntityVector const & GetEntities() const;

		void Purge();

	private:
		// variables
		EntityVector entities;
	};
}
