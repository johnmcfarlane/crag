/*
 *  PhysicalBody.cpp
 *  Crag
 *
 *  Created by John on 10/29/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#include "pch.h"

#include "PhysicalBody.h"
#include "Physics.h"


sim::PhysicalBody::PhysicalBody(Entity & entity, dGeomID init_geom)
: body(Physics::Get().CreateBody(entity))
, geom(init_geom)
{
	dGeomSetBody(geom, body);
	
	Physics::Get().AddPhysicalBody(* this);
}

sim::PhysicalBody::~PhysicalBody()
{
	Physics::Get().RemovePhysicalBody(* this);
	
	Physics::Destroy(geom);
	Physics::Destroy(body);
}


sim::PhysicalSphere::PhysicalSphere(Entity & entity, float density, float radius)
: PhysicalBody(entity, Physics::Get().CreateSphere(entity, density, radius))
{
}

float sim::PhysicalSphere::GetRadius() const
{
	return dGeomSphereGetRadius(GetGeom());
}
