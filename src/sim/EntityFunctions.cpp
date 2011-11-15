//
//  EntityFunctions.cpp
//  crag
//
//  Created by John McFarlane on 2011/11/15.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "EntityFunctions.h"

#include "EntitySet.h"
#include "Simulation.h"

#include "physics/Engine.h"


namespace sim
{
	
	// Perform a step in the simulation. 
	void TickEntities(Simulation & simulation)
	{
		EntitySet & entity_set = simulation.GetEntities();
		Entity::List & entities = entity_set.GetEntities();
		for (Entity::List::iterator it = entities.begin(), end = entities.end(); it != end; ++ it)
		{
			script::Object & object = * it;
			Entity & entity = static_cast<Entity &>(object);
			
			entity.Tick(simulation);
		}
	}

	void UpdateModels(EntitySet const & entity_set)
	{
		Entity::List const & entities = entity_set.GetEntities();
		for (Entity::List::const_iterator it = entities.begin(), end = entities.end(); it != end; ++ it)
		{
			script::Object const & object = * it;
			Entity const & entity = static_cast<Entity const &>(object);
			entity.UpdateModels();
		}
	}

	void AttachEntities(Uid const & uid1, Uid const & uid2, EntitySet & entity_set, physics::Engine & physics_engine)
	{
		Uid const uids[2] = { uid1, uid2 };
		physics::Body * bodies[2];
		
		for (int index = 0; index < 2; ++ index)
		{
			Entity * entity = entity_set.GetEntity(uids[index]);
			if (entity == nullptr)
			{
				return;
			}
			
			physics::Body * body = entity->GetBody();
			if (body == nullptr)
			{
				return;
			}
			
			bodies[index] = body;
		}
		
		physics_engine.Attach(ref(bodies[0]), ref(bodies[1]));
	}
}
