/*
 *  Entity.cpp
 *  Crag
 *
 *  Created by john on 5/21/09.
 *  Copyright 2009-2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Entity.h"

#include "Simulation.h"


using namespace sim;


//////////////////////////////////////////////////////////////////////
// Entity member definitions

Entity::Entity()
{
}

Entity::~Entity()
{
}

void Entity::Destroy(Entity & entity)
{
	// create message
	RemoveEntityMessage message = { entity };

	// set message
	Simulation::Daemon::SendMessage(message);
}

void Entity::Tick(Simulation & simulation)
{
}

void Entity::GetGravitationalForce(Vector3 const & /*pos*/, Vector3 & /*gravity*/) const
{
}

void Entity::UpdateModels() const
{
}

void Entity::SetPosition(Vector3 const & position)
{
	// For now, only observer can have its position set.
	Assert(false);
}
