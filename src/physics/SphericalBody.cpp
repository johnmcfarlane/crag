//
//  SphericalBody.cpp
//  crag
//
//  Created by John on 6/16/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "SphericalBody.h"

#include "Engine.h"

#include <ode/collision.h>
#include <ode/mass.h>
#include <ode/objects.h>

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// SphericalBody

SphericalBody::SphericalBody(Transformation const & transformation, Vector3 const * velocity, Engine & engine, Scalar radius)
: Body(transformation, velocity, engine, engine.CreateSphere(radius))
{
}

void SphericalBody::SetRadius(Scalar radius) const
{
	dGeomSphereSetRadius(GetCollisionHandle(), radius);
}

Scalar SphericalBody::GetRadius() const
{
	return dGeomSphereGetRadius(GetCollisionHandle());
}

void SphericalBody::SetDensity(Scalar density)
{
	ASSERT(GetBodyHandle() != 0);
	
	dMass m;
	Scalar radius = GetRadius();
	dMassSetSphere (& m, density, radius);
	dBodySetMass (GetBodyHandle(), & m);
}

bool SphericalBody::OnCollision(Body & body, ContactInterface & contact_interface)
{
	Sphere3 bounding_sphere(GetTranslation(), GetRadius());
	return body.OnCollisionWithSolid(* this, bounding_sphere, contact_interface);
}
