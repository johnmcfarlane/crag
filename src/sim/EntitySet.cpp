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

#include "Simulation.h"
#include "Entity.h"

#include "physics/Body.h"

#include "core/ConfigEntry.h"


using namespace sim;


namespace
{
	// TODO: This could cause the Observer to be destroyed
	CONFIG_DEFINE(purge_distance, double, 1000000000000.);
}


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
		script::Object & object = * it;
		Entity & entity = static_cast<Entity &>(object);

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

void EntitySet::Purge()
{
	for (Entity::List::iterator it = entities.begin(); it != entities.end(); ++ it) 
	{
		script::Object & object = * it;
		Entity & entity = static_cast<Entity &>(object);

		physics::Body const * body = entity.GetBody();
		if (body == nullptr)
		{
			continue;
		}

		Vector3 position = body->GetPosition();
		if (Length(position) < purge_distance)
		{
			continue;
		}

		// Invalid values of position, e.g. -1.#IND, will cause errno to become set.
		// Those sorts of errors are exactly what we're looking for in this function.
		errno = 0;

		std::cerr << "purging entity with bad position" << std::endl;
		delete body;
		entity.SetBody(nullptr);
	}

	// Make sure errno was unset by the end of this function.
	AssertErrno();
}
