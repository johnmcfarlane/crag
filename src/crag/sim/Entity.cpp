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


//////////////////////////////////////////////////////////////////////
// Entity member definitions

sim::Entity::Entity()
{
}

sim::Entity::~Entity()
{
}

void sim::Entity::Destroy(Entity & entity)
{
	// create message
	RemoveEntityMessage message = { entity };

	// set message
	Simulation::SendMessage(message, true);
}

void sim::Entity::Tick()
{
}

void sim::Entity::Draw(gfx::Pov const & pov) const
{
}

void sim::Entity::GetGravitationalForce(sim::Vector3 const & /*pos*/, sim::Vector3 & /*gravity*/) const
{
}

bool sim::Entity::GetRenderRange(Ray3 const & camera_ray, double * range, bool wireframe) const
{
	return false;
}

void sim::Entity::SetPosition(Vector3 const & position)
{
	// For now, only observer can have its position set.
	Assert(false);
}
