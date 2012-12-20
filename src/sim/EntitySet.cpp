//
//  EntitySet.cpp
//  crag
//
//  Created by john on 4/28/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "EntitySet.h"

#include "Engine.h"
#include "Entity.h"

#include "physics/Body.h"


using namespace sim;


//////////////////////////////////////////////////////////////////////
// EntitySet definitions


EntitySet::~EntitySet()
{
	// it's likely that the main applet failed really badly and didn't clean up
	ASSERT(entities.empty());
}

void EntitySet::Add(Entity & entity)
{
	ASSERT(! entities.contains(entity));
	entities.push_back(entity);
}

void EntitySet::Remove(Entity & entity)
{
	ASSERT(entities.contains(entity));
	entities.remove(entity);
}

Entity * EntitySet::GetEntity(smp::Uid uid)
{
	for (Entity::List::iterator it = entities.begin(); it != entities.end(); ++ it)
	{
		Entity & entity = * it;

		if (entity.GetUid() == uid)
		{
			return & entity;
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
