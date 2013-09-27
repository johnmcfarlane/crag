//
//  scripts/ga/Sensor.cpp
//  crag
//
//  Created by John McFarlane on 2013-03-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Sensor.h"

#include "AnimatController.h"

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "form/RayCastResult.h"

#include "physics/RayCast.h"

#include "gfx/Debug.h"

#include "core/Random.h"
#include "core/Roster.h"

using namespace sim;

namespace
{
	// given a position which is relative to entity, returns simulation-relative position
	Vector3 TransformPosition(Vector3 local, Entity const & entity)
	{
		auto location = entity.GetLocation();
		return location->Transform(local);
	}
	
	// given a position which is relative to entity, returns simulation-relative position
	Vector3 TransformDirection(Vector3 local, Entity const & entity)
	{
		auto location = entity.GetLocation();
		return location->Rotate(local);
	}

	Ray3 Transform(Ray3 const & local, Entity const & entity)
	{
		Ray3 global;
	
		global.position = TransformPosition(local.position, entity);
		global.direction = TransformDirection(local.direction, entity);

		// TODO: broken assert (hopefully) because execution gets here ahead of an origin		
		//ASSERT(geom::Length(TransformPosition(geom::Project(local, 1.0f), entity) - geom::Project(global, 1.0f)) < .0001f);
		
		return global;
	}
	Vector3 GetRandomDirection(Random & sequence)
	{
		Vector3 random_direction;
		while (true)
		{
			random_direction.x = sequence.GetUnitInclusive<Scalar>() - 0.5f;
			random_direction.y = sequence.GetUnitInclusive<Scalar>() - 0.5f;
			random_direction.z = sequence.GetUnitInclusive<Scalar>() - 0.5f;
			auto length_squared = geom::LengthSq(random_direction);
			if (length_squared > 1.0f)
			{
				continue;
			}
			
			auto inverse_length = FastInvSqrt(length_squared);
			random_direction *= inverse_length;
			ASSERT(NearEqual(geom::Length(random_direction), 1.f, 0.001f));
			
			return random_direction;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// sim::Sensor member definitions

DEFINE_POOL_ALLOCATOR(Sensor, 800);

Sensor::Sensor(Entity & entity, Ray3 const & ray, Scalar length, Scalar variance)
: _entity(entity)
, _length(length)
, _variance(variance)
, _ray_cast(ref(new physics::RayCast(entity.GetEngine().GetPhysicsEngine(), length)))
, _local_ray(ray)
{
	GenerateScanRay();
	
	auto location = entity.GetLocation();
	auto body = location->GetBody();
	_ray_cast.SetIsCollidable(* body, false);
	
	auto & roster = GetTickRoster();
	roster.AddOrdering(& Sensor::Tick, & AnimatController::Tick);
	roster.AddCommand(* this, & Sensor::Tick);
}

Sensor::~Sensor()
{
	auto & roster = GetTickRoster();
	roster.RemoveCommand(* this, & Sensor::Tick);

	delete & _ray_cast;
}

Scalar Sensor::GetReading() const
{
	const auto& result = _ray_cast.GetResult();
	if (! result)
	{
		return 1.f;
	}
	
	auto contact_distance = result.GetDistance();
	VerifyOp (contact_distance, <=, _length);
	
	auto ratio = contact_distance / _length;
	ASSERT(ratio >= 0);
	ASSERT(ratio <= 1.f);
	
	return ratio;
}

#if defined(VERIFY)
void Sensor::Verify() const
{
	VerifyObject(_local_ray.position);
	VerifyIsUnit(_local_ray.direction, .0001f);
}
#endif

void Sensor::Tick()
{
	// draw previous ray result
	auto reading = GetReading();
	auto scan_ray = _ray_cast.GetRay();
	auto start = scan_ray.position;
	auto end = geom::Project(scan_ray, _length);
	if (reading < 1)
	{
		auto penetration_position = geom::Project(scan_ray, reading * _length);
		gfx::Debug::AddLine(start, penetration_position, gfx::Debug::Color::Yellow());
		gfx::Debug::AddLine(penetration_position, end, gfx::Debug::Color::Red());
	}
	else
	{
		gfx::Debug::AddLine(start, end, gfx::Debug::Color::Green());
	}

	GenerateScanRay();
}

Ray3 Sensor::GetGlobalRay() const
{
	return Transform(_local_ray, _entity);
}

void Sensor::GenerateScanRay() const
{
	VerifyObject(* this);
	
	Ray3 scan_ray = GetGlobalRay();
	VerifyIsUnit(scan_ray.direction, .0001f);

	scan_ray.direction *= _length;
	
	auto random_direction = GetRandomDirection(Random::sequence);
	scan_ray.direction += random_direction * _length * _variance;
	
	auto scan_length = geom::Length(scan_ray.direction);
	scan_ray.direction /= scan_length;
	
	// generate new ray
	_ray_cast.SetRay(scan_ray);
}

core::locality::Roster & Sensor::GetTickRoster()
{
	return _entity.GetEngine().GetTickRoster();
}
