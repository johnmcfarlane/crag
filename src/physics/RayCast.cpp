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
{
	// register for physics tick
	auto & roster = _engine.GetPreTickRoster();
	roster.AddCommand(* this, & RayCast::ResetResult);
}

RayCast::~RayCast()
{
	// register for physics tick
	auto & roster = _engine.GetPreTickRoster();
	roster.RemoveCommand(* this, & RayCast::ResetResult);
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(RayCast, self)
	CRAG_VERIFY(self._result);
	
	// test length
	dReal length = self.GetLength();
	CRAG_VERIFY_OP(length, >=, 0);
	CRAG_VERIFY_EQUAL(length, length);

	// test position/direction
	dVector3 position;
	dVector3 direction;
	dGeomRayGet(self.GetCollisionHandle(), position, direction);
	Ray3 ray(
		physics::Convert(position),
		physics::Convert(direction)
	);
	CRAG_VERIFY(ray);
	CRAG_VERIFY_TRUE(NearEqual(geom::Length(ray), 1, .0001f));
CRAG_VERIFY_INVARIANTS_DEFINE_END

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
	CRAG_VERIFY_UNIT(ray, .0001f);
	CRAG_VERIFY(* this);

	dGeomRaySet(GetCollisionHandle(), 
		ray.position.x, ray.position.y, ray.position.z, 
		ray.direction.x, ray.direction.y, ray.direction.z);

	CRAG_VERIFY_NEARLY_EQUAL(geom::Length(ray.position - GetRay().position), 0.f, 0.01f);
	CRAG_VERIFY_NEARLY_EQUAL(geom::Length(geom::Normalized(ray.direction) - geom::Normalized(GetRay().direction)), 0.f, 0.001f);
	CRAG_VERIFY_NEARLY_EQUAL_LOG(geom::Length(ray.direction), geom::Length(GetRay().direction), 0.001f);
	CRAG_VERIFY(* this);
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

form::RayCastResult const & RayCast::GetResult() const
{
	return _result;
}

void RayCast::SampleResult(form::RayCastResult const & result)
{
	CRAG_VERIFY(result);
	CRAG_VERIFY(* this);
	
	_result = std::min(_result, result);
	
	CRAG_VERIFY(* this);
}

void RayCast::ResetResult()
{
	_result = form::RayCastResult();
}

void RayCast::SetDensity(Scalar)
{
	ASSERT(false);
}

bool RayCast::OnCollision(Body & body, ContactInterface &)
{
	return body.OnCollisionWithRay(* this);
}
