//
//  BoxBody.cpp
//  crag
//
//  Created by John McFarlane on 9/5/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "BoxBody.h"

#include "Engine.h"
#include "SphericalBody.h"

#include <ode/collision.h>
#include <ode/mass.h>
#include <ode/objects.h>

////////////////////////////////////////////////////////////////////////////////
// BoxBody

physics::BoxBody::BoxBody(Transformation const & transformation, Vector3 const * velocity, Engine & engine, Vector3 const & dimensions)
: Body(transformation, velocity, engine, engine.CreateBox(dimensions))
{
}

void physics::BoxBody::SetDimensions(Vector3 const & dimensions) const
{
	dGeomBoxSetLengths(geom_id, dimensions.x, dimensions.y, dimensions.z);
}

physics::Vector3 physics::BoxBody::GetDimensions() const
{
	Vector3 dimensions;
	dGeomBoxGetLengths(geom_id, dimensions.GetAxes());
	return dimensions;
}

void physics::BoxBody::SetDensity(Scalar density)
{
	ASSERT(body_id != 0);
	
	dMass m;
	Vector3 dimensions = GetDimensions();
	dMassSetBox (& m, density, dimensions.x, dimensions.y, dimensions.z);
	dBodySetMass (body_id, & m);
}

void physics::BoxBody::OnDeferredCollisionWithPlanet(Body const & planet, IntersectionFunctor & functor) const
{
	planet.OnDeferredCollisionWithBox(* this, functor);
}
