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

#include "core/RosterObjectDefine.h"

#include "gfx/Debug.h"

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// physics::RayCast member definitions

RayCast::RayCast(Engine & engine, Scalar length)
: Body(Transformation(), nullptr, engine, engine.CreateRay(length))
{
}

CRAG_ROSTER_OBJECT_DEFINE(
	RayCast,
	2000,
	Pool::Call<& RayCast::ResetResult>(Engine::GetPreTickRoster()))

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
	CRAG_VERIFY_TRUE(NearEqual(geom::Magnitude(ray), 1, .0001f));
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

void RayCast::SetRay(Ray3 const & ray)
{
	CRAG_VERIFY_UNIT(ray, .0001f);
	CRAG_VERIFY(* this);

	dGeomRaySet(GetCollisionHandle(), 
		ray.position.x, ray.position.y, ray.position.z, 
		ray.direction.x, ray.direction.y, ray.direction.z);

	CRAG_VERIFY_NEARLY_EQUAL(geom::Magnitude(ray.position - GetRay().position), 0.f, 0.01f);
	CRAG_VERIFY_UNIT(ray.direction, .0001f);
	CRAG_VERIFY_UNIT(GetRay().direction, .0001f);
	CRAG_VERIFY_NEARLY_EQUAL(geom::Magnitude(ray.direction - GetRay().direction), 0.f, 0.001f);
	CRAG_VERIFY_NEARLY_EQUAL_LOG(geom::Magnitude(ray.direction), geom::Magnitude(GetRay().direction), 0.001f);
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

void RayCast::DebugDraw() const
{
	// draw previous ray result
	auto scan_ray = GetRay();
	auto start = scan_ray.position;
	auto end = geom::Project(scan_ray, GetLength());
	if (_result)
	{
		auto distance = _result.GetDistance();
		auto penetration_position = geom::Project(scan_ray, distance);
		gfx::Debug::AddLine(start, penetration_position, gfx::Debug::Color::Yellow());
		gfx::Debug::AddLine(penetration_position, end, gfx::Debug::Color::Red());
	}
	else
	{
		gfx::Debug::AddLine(start, end, gfx::Debug::Color::Green());
	}
}

void RayCast::SetDensity(Scalar)
{
	ASSERT(false);
}

bool RayCast::HandleCollision(Body & body, ContactFunction &)
{
	return body.HandleCollisionWithRay(* this);
}

#if defined(CRAG_DEBUG)
void RayCast::OnContact(Body &)
{
	DEBUG_BREAK("Rays are not meant to collide with anything");
}
#endif
