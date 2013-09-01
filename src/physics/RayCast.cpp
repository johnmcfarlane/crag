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

#include <ode/collision.h>

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// physics::RayCast member definitions

RayCast::RayCast(Engine & engine)
: Body(Matrix44::Identity(), nullptr, engine, engine.CreateRay(0))
{
}

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

void RayCast::SetRay(Ray3 ray)
{
	auto length_squared = geom::LengthSq(ray);

	decltype(length_squared) length;
	if (length_squared == 0)
	{
		length = 0;
		ray.direction = Vector3(1,0,0);
	}
	else
	{
		length = std::sqrt(length_squared);
		ray.direction /= length;
	}

	dGeomRaySetLength(GetCollisionHandle(), length);
	dGeomRaySet(GetCollisionHandle(), 
		ray.position.x, ray.position.y, ray.position.z, 
		ray.direction.x, ray.direction.y, ray.direction.z);
}

Ray3 RayCast::GetRay() const
{
	Ray3 ray;
	dGeomRayGet(GetCollisionHandle(), & ray.position[0], & ray.direction[0]);
	ray.direction *= GetLength();
	return ray;
}

Scalar RayCast::GetLength() const
{
	return dGeomRayGetLength(GetCollisionHandle());
}

void RayCast::SetDensity(Scalar)
{
	ASSERT(false);
}

bool RayCast::OnCollision(Body const & body) const
{
	return body.OnCollisionWithRay(* this, GetRay());
}

#if defined(VERIFY)
void RayCast::Verify() const
{
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
