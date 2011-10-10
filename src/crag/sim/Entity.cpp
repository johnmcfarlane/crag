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

#include "script/MetaClass.h"


using namespace sim;


namespace
{
	PyObject * entity_set_collidable(PyObject * self, PyObject * args)
	{
		int collision;
		if (! PyArg_ParseTuple(args, "i", & collision))
		{
			return nullptr;
		}
		
		sim::Entity & entity = Entity::GetRef(self);
		entity.SetIsCollidable(collision != 0);
		
		Py_RETURN_NONE;
	}
}


DEFINE_SCRIPT_CLASS_BEGIN(sim, Entity)
	SCRIPT_CLASS_METHOD("set_collidable", entity_set_collidable, "Set whether entity collides with other entities")
DEFINE_SCRIPT_CLASS_END


//////////////////////////////////////////////////////////////////////
// Entity member definitions


Entity::Entity()
{
}

Entity::~Entity()
{
}

void Entity::Create(Entity & entity, PyObject & args)
{
	// TODO: Make it so this function isn't even defined.
	Assert(false);
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

void Entity::SetIsCollidable(bool collision)
{
	Assert(! collision)
}

bool Entity::GetCollision() const
{
	return false;
}
