/*
 *  EntitySet.cpp
 *  Crag
 *
 *  Created by john on 4/28/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "EntitySet.h"

#include "Simulation.h"
#include "Entity.h"

#include "physics/Body.h"


using namespace sim;

//////////////////////////////////////////////////////////////////////
// EntitySet definitions


EntitySet::~EntitySet()
{
	// it's likely that the script failed really badly and didn't clean up
	Assert(entities.empty());
}

void EntitySet::Add(Entity & entity)
{
	Assert(! entities.contains(entity));
	entities.push_back(entity);
}

void EntitySet::Remove(Entity & entity)
{
	Assert(entities.contains(entity));
	entities.remove(entity);
}

Entity * EntitySet::GetEntity(smp::Uid uid)
{
	for (Entity::List::iterator it = entities.begin(); it != entities.end(); ++ it)
	{
		Entity & e = * it;
		if (e.GetUid() == uid)
		{
			return & e;
		}
	}
	
	return nullptr;
}

Entity::List & EntitySet::GetEntities()
{
	return entities;
}

Entity::List const & EntitySet::GetEntities() const
{
	return entities;
}

void EntitySet::Purge()
{
	for (Entity::List::iterator it = entities.begin(); it != entities.end(); ++ it) 
	{
		Entity & e = * it;
		physics::Body const * body = e.GetBody();
		if (body == nullptr)
		{
			continue;
		}

		Vector3 position = body->GetPosition();
		if (position == position)
		{
			continue;
		}

		std::cerr << "purging entity with bad position" << std::endl;
		delete body;
		e.SetBody(nullptr);
	}
}
