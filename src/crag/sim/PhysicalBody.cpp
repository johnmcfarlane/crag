/*
 *  PhysicalBody.cpp
 *  Crag
 *
 *  Created by John on 10/29/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "PhysicalBody.h"
#include "Physics.h"


sim::PhysicalBody::PhysicalBody()
: body(nullptr)
, geom(nullptr)
{
}

sim::PhysicalBody::~PhysicalBody()
{
	Deinit();
}

void sim::PhysicalBody::Init(Entity & entity, dGeomID init_geom)
{
	Create(entity, init_geom);
	Physics::Get().AddPhysicalBody(* this);
}

void sim::PhysicalBody::Deinit()
{
	Destroy();
	Physics::Get().RemovePhysicalBody(* this);
}

void sim::PhysicalBody::Create(Entity & entity, dGeomID init_geom)
{
	Destroy();

	body = Physics::Get().CreateBody(entity);
	geom = init_geom;

	dGeomSetBody(geom, body);
}

void sim::PhysicalBody::Destroy()
{
	if (geom != nullptr)
	{
		Physics::Destroy(geom);
		geom = nullptr;
	}

	if (body != nullptr)
	{
		Physics::Destroy(body);
		body = nullptr;
	}
}


////////////////////////////////////////////////////////////////////////////////
// PhysicalSphere

sim::PhysicalSphere::PhysicalSphere()
{
}

void sim::PhysicalSphere::Init(Entity & entity, float density, float radius)
{
	PhysicalBody::Init(entity, Physics::Get().CreateSphere(entity, density, radius));
}

sim::Scalar sim::PhysicalSphere::GetRadius() const
{
	return dGeomSphereGetRadius(GetGeom());
}
