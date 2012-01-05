//
//  IntersectionFunctor.cpp
//  crag
//
//  Created by John McFarlane on 9/13/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "IntersectionFunctor.h"

#include "Engine.h"

#include "sim/Simulation.h"

#include "core/ConfigEntry.h"


namespace
{
	CONFIG_DEFINE(planet_collision_friction, physics::Scalar, .1);	// coulomb friction coefficient
	CONFIG_DEFINE(planet_collision_bounce, physics::Scalar, .50);
}


using namespace physics;


IntersectionFunctor::IntersectionFunctor(physics::Engine & physics_engine, dGeomID object_geom, dGeomID planet_geom)
: _physics_engine(physics_engine)
{
	ZeroObject(_contact);
	
	_contact.surface.mode = dContactBounce | dContactSlip1 | dContactSlip2;
	
	_contact.surface.mu = planet_collision_friction;
	_contact.surface.bounce = planet_collision_bounce;
	_contact.surface.bounce_vel = .1f;
	_contact.geom.g1 = object_geom;
	_contact.geom.g2 = planet_geom;
}

void IntersectionFunctor::operator()(sim::Vector3 const & pos, sim::Vector3 const & normal, sim::Scalar depth)
{
	_contact.geom.pos[0] = pos.x;
	_contact.geom.pos[1] = pos.y;
	_contact.geom.pos[2] = pos.z;
	_contact.geom.normal[0] = normal.x;
	_contact.geom.normal[1] = normal.y;
	_contact.geom.normal[2] = normal.z;
	_contact.geom.depth = depth;
	
	_physics_engine.OnContact(_contact);
}
