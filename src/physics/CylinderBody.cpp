//
//  CylinderBody.cpp
//  crag
//
//  Created by John on 2014-03-23.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "CylinderBody.h"

#include "Engine.h"

#include <ode/collision.h>
#include <ode/objects.h>

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// CylinderBody

CylinderBody::CylinderBody(Transformation const & transformation, Vector3 const * velocity, Engine & engine, Scalar radius, Scalar length)
: Body(transformation, velocity, engine, engine.CreateCylinder(radius, length))
{
}

void CylinderBody::SetParams(Scalar radius, Scalar length) const
{
	dGeomCylinderSetParams(GetCollisionHandle(), radius, length);
}

void CylinderBody::GetParams(Scalar & radius, Scalar & length) const
{
	return dGeomCylinderGetParams(GetCollisionHandle(), & radius, & length);
}

void CylinderBody::SetDensity(Scalar density)
{
	ASSERT(GetBodyHandle() != 0);
	
	dMass m;
	Scalar radius, length;
	GetParams(radius, length);
	dMassSetCylinder (& m, density, 0, radius, length);
	dBodySetMass (GetBodyHandle(), & m);
}

bool CylinderBody::OnCollision(Body & body, ContactInterface & contact_interface)
{
	Scalar radius, length;
	GetParams(radius, length);
	auto bounding_radius = geom::Magnitude(Vector3(radius, length, length));

	Sphere3 bounding_sphere(GetTranslation(), bounding_radius);
	return body.OnCollisionWithSolid(* this, bounding_sphere, contact_interface);
}
