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

////////////////////////////////////////////////////////////////////////////////
// SphericalBody

physics::SphericalBody::SphericalBody(Transformation const & transformation, Vector3 const * velocity, physics::Engine & engine, Scalar radius)
: Body(transformation, velocity, engine, engine.CreateSphere(radius))
{
}

void physics::SphericalBody::SetRadius(Scalar radius) const
{
	dGeomSphereSetRadius(_collision_handle, radius);
}

physics::Scalar physics::SphericalBody::GetRadius() const
{
	return dGeomSphereGetRadius(_collision_handle);
}

void physics::SphericalBody::SetDensity(Scalar density)
{
	ASSERT(_body_handle != 0);
	
	dMass m;
	Scalar radius = GetRadius();
	dMassSetSphere (& m, density, radius);
	dBodySetMass (_body_handle, & m);
}

void physics::SphericalBody::OnDeferredCollisionWithPlanet(Body const & planet, IntersectionFunctorRef const & functor) const
{
	planet.OnDeferredCollisionWithSphere(* this, functor);
}
