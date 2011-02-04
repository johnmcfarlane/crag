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

sim::Entity::Entity(SimulationPtr const & s)
{
	s->AddEntity(* this);
}

sim::Entity::~Entity()
{
	SimulationPtr s(Simulation::GetPtr());
	s->RemoveEntity(* this);
}

void sim::Entity::Tick(Universe const & universe)
{
}

void sim::Entity::Draw() const
{
}

void sim::Entity::GetGravitationalForce(sim::Vector3 const & /*pos*/, sim::Vector3 & /*gravity*/) const
{
}

bool sim::Entity::GetRenderRange(Ray3 const & camera_ray, double * range, bool wireframe) const
{
	return false;
}

physics::Body * sim::Entity::GetBody()
{
	return nullptr;
}

physics::Body const * sim::Entity::GetBody() const
{
	return nullptr;
}

#if DUMP
DumpStream & operator << (DumpStream & lhs, Entity & rhs)
{
	return lhs;
}
#endif
