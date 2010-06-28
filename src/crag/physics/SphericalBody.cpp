/*
 *  SphericalBody.cpp
 *  crag
 *
 *  Created by John on 6/16/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "SphericalBody.h"

#include "Singleton.h"


////////////////////////////////////////////////////////////////////////////////
// SphericalBody

physics::SphericalBody::SphericalBody(bool movable, Scalar radius)
: Body(dCreateSphere(Singleton::Get().space, radius), movable)
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

bool physics::SphericalBody::OnCollision(Body & that_body)
{
	return that_body.OnCollisionWithSphericalBody(* this, geom_id);
}

bool physics::SphericalBody::OnCollisionWithSphericalBody(SphericalBody & that_sphere, dGeomID that_geom_id)
{
	Assert(that_geom_id == that_sphere.geom_id);
	
	// There is no special code for sphere-sphere collision. 
	Singleton::Get().OnCollision(geom_id, that_geom_id);
	return true;
}
