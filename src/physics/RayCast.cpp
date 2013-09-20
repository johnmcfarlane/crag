//
//  RayCast.cpp
//  crag
//
//  Created by John on 2013-04-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "RayCast.h"
#include "Engine.h"

#include "geom/Matrix33.h"

#include "core/Roster.h"

#include <ode/collision.h>

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// physics::RayCast member definitions

RayCast::RayCast(Engine & engine, Scalar length)
: Body(Matrix44::Identity(), nullptr, engine, engine.CreateRay(length))
, _contact_distance(0)
{
	// register for physics tick
	auto & roster = _engine.GetPreTickRoster();
	roster.AddCommand(* this, & RayCast::ResetContactDistance);
}

RayCast::~RayCast()
{
	// register for physics tick
	auto & roster = _engine.GetPreTickRoster();
	roster.RemoveCommand(* this, & RayCast::ResetContactDistance);
}

#if defined(VERIFY)
void RayCast::Verify() const
{
	VerifyOp(_contact_distance, >=, 0);
	
	// test length
	dReal length = GetLength();
	VerifyOp(length, >=, 0);
	VerifyEqual(length, length);

	// test position/direction
	dVector3 position;
	dVector3 direction;
	dGeomRayGet(GetCollisionHandle(), position, direction);
	Ray3 ray(
		Vector3(position[0], position[1], position[2]),
		Vector3(direction[0], direction[1], direction[2])
	);
	VerifyObject(ray);
	VerifyTrue(NearEqual(geom::Length(ray), 1, .0001f));
}
#endif

void RayCast::SetDirection(Vector3 const & direction)
{
	Ray3 ray = GetRay();
	ray.direction = direction;
	SetRay(ray);
}

Vector3 RayCast::GetDirection() const
{
	Ray3 ray = GetRay();
	return ray.direction;
}

void RayCast::SetRay(Ray3 const ray)
{
	VerifyIsUnit(ray, .0001f);
	VerifyObject(* this);

	dGeomRaySet(GetCollisionHandle(), 
		ray.position.x, ray.position.y, ray.position.z, 
		ray.direction.x, ray.direction.y, ray.direction.z);

	ResetContactDistance();

	VerifyNearlyEqual(geom::Length(ray.position - GetRay().position), 0.f, 0.01f);
	VerifyNearlyEqual(geom::Length(geom::Normalized(ray.direction) - geom::Normalized(GetRay().direction)), 0.f, 0.001f);
	VerifyNearlyEqualLog(geom::Length(ray.direction), geom::Length(GetRay().direction), 0.001f);
	VerifyObject(* this);
}

Ray3 RayCast::GetRay() const
{
	Ray3 ray;
	dGeomRayGet(GetCollisionHandle(), & ray.position[0], & ray.direction[0]);
	return ray;
}

void RayCast::SetLength(Scalar length)
{
	dGeomRaySetLength(GetCollisionHandle(), length);
}

Scalar RayCast::GetLength() const
{
	return dGeomRayGetLength(GetCollisionHandle());
}

bool RayCast::IsContacted() const
{
	VerifyObject(* this);
	
	return _contact_distance < max_contact_distance;
}

Scalar RayCast::GetContactDistance() const
{
	ASSERT(IsContacted());
	
	return _contact_distance;
}

void RayCast::SampleContact(Scalar contact_distance)
{
	VerifyTrue(contact_distance >= 0);
	VerifyObject(* this);
	
	_contact_distance = std::min(_contact_distance, contact_distance);
	
	VerifyObject(* this);
}

void RayCast::ResetContactDistance()
{
	_contact_distance = std::numeric_limits<Scalar>::max();
}

void RayCast::SetDensity(Scalar)
{
	ASSERT(false);
}

bool RayCast::OnCollision(Body & body)
{
	return body.OnCollisionWithRay(* this);
}
