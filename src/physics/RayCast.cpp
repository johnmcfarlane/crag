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

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// physics::RayCast member definitions

#if defined(VERIFY)
void RayCast::Verify() const
{
	// test length
	dReal length = dGeomRayGetLength(_collision_handle);
	VerifyOp(length, >=, 0);
	VerifyEqual(length, length);

	// test position/direction
	dVector3 position;
	dVector3 direction;
	dGeomRayGet(_collision_handle, position, direction);
	Ray3 ray(
		Vector3(position[0], position[1], position[2]),
		Vector3(direction[0], direction[1], direction[2])
	);
	VerifyObject(ray);
	VerifyTrue(NearEqual(geom::Length(ray), 1, .0001f));
}
#endif

RayCast::RayCast(Engine & engine)
: Body(engine, engine.CreateRay(0), false)
{
}

void RayCast::setDirection(Vector3 const & direction)
{
	Ray3 ray = getRay();
	ray.direction = direction;
	setRay(ray);
}

Vector3 RayCast::getDirection() const
{
	Ray3 ray = getRay();
	return ray.direction;
}

void RayCast::setRay(Ray3 ray)
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

	dGeomRaySetLength(_collision_handle, length);
	dGeomRaySet(_collision_handle, 
		ray.position.x, ray.position.y, ray.position.z, 
		ray.direction.x, ray.direction.y, ray.direction.z);
}

Ray3 RayCast::getRay() const
{
	Ray3 ray;
	dGeomRayGet(_collision_handle, & ray.position[0], & ray.direction[0]);
	ray.direction *= dGeomRayGetLength(_collision_handle);
	return ray;
}

Scalar RayCast::GetLength() const
{
	return dGeomRayGetLength(_collision_handle);
}

Vector3 RayCast::GetScale() const
{
	return Vector3(GetLength(),0,0);
}

void RayCast::SetDensity(Scalar density)
{
	ASSERT(false);
}

void RayCast::OnDeferredCollisionWithPlanet(Body const & planet, IntersectionFunctor & functor) const
{
	DEBUG_BREAK("Not yet implemented");
	//planet.OnDeferredCollisionWithRay(* this, functor);
}
