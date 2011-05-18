/*
 *  SphericalBody.cpp
 *  crag
 *
 *  Created by John on 6/16/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "SphericalBody.h"

#include "Engine.h"


////////////////////////////////////////////////////////////////////////////////
// SphericalBody

physics::SphericalBody::SphericalBody(physics::Engine & physics_engine, bool movable, Scalar radius)
: Body(physics_engine, dCreateSphere(physics_engine.space, radius), movable)
{
}

void physics::SphericalBody::SetRadius(Scalar radius) const
{
	dGeomSphereSetRadius(geom_id, radius);
}

physics::Scalar physics::SphericalBody::GetRadius() const
{
	return dGeomSphereGetRadius(geom_id);
}

void physics::SphericalBody::SetDensity(Scalar density)
{
	Assert(body_id != 0);
	
	dMass m;
	Scalar radius = GetRadius();
	dMassSetSphere (& m, density, radius);
	dBodySetMass (body_id, & m);
}

bool physics::SphericalBody::OnCollision(Engine & engine, Body & that_body)
{
	return that_body.OnCollisionWithSphericalBody(engine, * this, geom_id);
}

bool physics::SphericalBody::OnCollisionWithSphericalBody(Engine & engine, SphericalBody & that_sphere, dGeomID that_geom_id)
{
	Assert(that_geom_id == that_sphere.geom_id);
	
	// There is no special code for sphere-sphere collision. 
	engine.OnCollision(geom_id, that_geom_id);
	return true;
}
