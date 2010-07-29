/*
 *  PlanetCollisionFunctor.cpp
 *  crag
 *
 *  Created by John on 6/29/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "PlanetCollisionFunctor.h"

#include "physics/Singleton.h"

#include "core/ConfigEntry.h"


namespace  
{
	CONFIG_DEFINE(planet_collision_friction, physics::Scalar, .05);	// coulomb friction coefficient
	CONFIG_DEFINE(planet_collision_bounce, physics::Scalar, .25);
}


////////////////////////////////////////////////////////////////////////////////
// PlanetCollisionFunctor members

sim::PlanetCollisionFunctor::PlanetCollisionFunctor(form::Formation const & in_planet_formation, dGeomID in_planet_geom, dGeomID in_object_geom)
: planet_formation(in_planet_formation)
{
	contact.geom.g1 = in_planet_geom;
	contact.geom.g2 = in_object_geom;
}

void sim::PlanetCollisionFunctor::OnContact(Vector3 const & pos, Vector3 const & normal, Scalar depth)
{
	contact.surface.mode = dContactBounce /*| dContactSoftCFM*/;
	contact.surface.mu = planet_collision_friction;				// used (by default)
	contact.surface.bounce = planet_collision_bounce;		// used
	contact.surface.bounce_vel = .1f;	// used
	contact.geom.pos[0] = pos.x;
	contact.geom.pos[1] = pos.y;
	contact.geom.pos[2] = pos.z;
	contact.geom.normal[0] = normal.x;
	contact.geom.normal[1] = normal.y;
	contact.geom.normal[2] = normal.z;
	contact.geom.depth = depth;
	
	physics::Singleton & singleton = physics::Singleton::Get();
	singleton.OnContact(contact, contact.geom.g1, contact.geom.g2);
}

