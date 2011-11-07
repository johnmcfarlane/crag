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
	Assert(std::find(entities.begin(), entities.end(), & entity) == entities.end());
	entities.push_back(& entity);
}

void EntitySet::Remove(Entity & entity)
{
	EntityVector::iterator i = std::find(entities.begin(), entities.end(), & entity);
	Assert(i != entities.end());
	entities.erase(i);
	
	Assert(std::find(entities.begin(), entities.end(), & entity) == entities.end());
}

Entity * EntitySet::GetEntity(smp::Uid uid)
{
	for (EntityVector::const_iterator it = entities.begin(); it != entities.end(); ++ it)
	{
		Entity & e = * * it;
		if (e.GetUid() == uid)
		{
			return & e;
		}
	}
	
	return nullptr;
}

EntityVector & EntitySet::GetEntities()
{
	return entities;
}

EntityVector const & EntitySet::GetEntities() const
{
	return entities;
}

void EntitySet::Purge()
{
	for (EntityVector::const_iterator it = entities.begin(); it != entities.end(); ++ it) 
	{
		Entity & e = * * it;
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
