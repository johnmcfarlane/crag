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


////////////////////////////////////////////////////////////////////////////////
// SphericalBody

physics::SphericalBody::SphericalBody(physics::Engine & engine, bool movable, Scalar radius)
: Body(engine, engine.CreateSphere(radius), movable)
{
}

physics::Vector3 physics::SphericalBody::GetScale() const
{
	Scalar radius = GetRadius();
	return Vector3(radius, radius, radius);
}

void physics::SphericalBody::SetRadius(Scalar radius) const
{
	dGeomSphereSetRadius(_collision_handle, radius);
}

physics::Scalar physics::SphericalBody::GetRadius() const
{
#if defined(USE_ODE)
	return dGeomSphereGetRadius(_collision_handle);
#endif

#if defined(USE_BULLET)
	DEBUG_MESSAGE("not implemented");
#endif
}

void physics::SphericalBody::SetDensity(Scalar density)
{
	ASSERT(_body_handle != 0);
	
	dMass m;
	Scalar radius = GetRadius();
	dMassSetSphere (& m, density, radius);
	dBodySetMass (_body_handle, & m);
}

void physics::SphericalBody::OnDeferredCollisionWithPlanet(Body const & planet, IntersectionFunctor & functor) const
{
	planet.OnDeferredCollisionWithSphere(* this, functor);
}
