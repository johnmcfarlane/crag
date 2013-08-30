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

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// BoxBody

BoxBody::BoxBody(Transformation const & transformation, Vector3 const * velocity, Engine & engine, Vector3 const & dimensions)
: Body(transformation, velocity, engine, engine.CreateBox(dimensions))
{
}

void BoxBody::SetDimensions(Vector3 const & dimensions) const
{
	dGeomBoxSetLengths(GetCollisionHandle(), dimensions.x, dimensions.y, dimensions.z);
}

Vector3 BoxBody::GetDimensions() const
{
	Vector3 dimensions;
	dGeomBoxGetLengths(GetCollisionHandle(), dimensions.GetAxes());
	return dimensions;
}

void BoxBody::SetDensity(Scalar density)
{
	ASSERT(GetCollisionHandle() != nullptr);
	
	dMass m;
	Vector3 dimensions = GetDimensions();
	dMassSetBox (& m, density, dimensions.x, dimensions.y, dimensions.z);
	dBodySetMass (GetBodyHandle(), & m);
}

Sphere3 BoxBody::GetBoundingSphere() const
{
	Sphere3 bounding_sphere;
	
	bounding_sphere.center = GetTranslation();
	
	Vector3 dimensions = GetDimensions();
	Vector3 extents = dimensions * Scalar(.5);
	bounding_sphere.radius = geom::Length(extents);
	
	return bounding_sphere;
}

void BoxBody::OnDeferredCollisionWithPlanet(Body const & planet, IntersectionFunctorRef const & functor) const
{
	planet.OnDeferredCollisionWithBox(* this, functor);
}
