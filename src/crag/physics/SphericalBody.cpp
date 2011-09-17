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
//#include "IntersectionFunctor.h"
//#include "PlanetaryBody.h"
//
//#include "form/FormationManager.h"
//#include "form/Formation.h"
//#include "form/node/NodeBuffer.h"
//#include "form/scene/ForEachIntersection.h"


////////////////////////////////////////////////////////////////////////////////
// SphericalBody

physics::SphericalBody::SphericalBody(Engine & engine, bool movable, Scalar radius)
: Body(engine, dCreateSphere(engine.space, radius), movable)
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

void physics::SphericalBody::OnDeferredCollisionWithPlanet(Body const & planet, IntersectionFunctor & functor) const
{
	planet.OnDeferredCollisionWithSphere(* this, functor);
}
