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
			random_direction.x = sequence.GetUnitInclusive<Scalar>() - 0.5;
			random_direction.y = sequence.GetUnitInclusive<Scalar>() - 0.5;
			random_direction.z = sequence.GetUnitInclusive<Scalar>() - 0.5;
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

DEFINE_POOL_ALLOCATOR(Sensor, 80);

Sensor::Sensor(Entity & entity, Ray3 const & ray)
: _entity(entity)
, _length(geom::Length(ray.direction))
, _ray_cast(ref(new physics::RayCast(entity.GetEngine().GetPhysicsEngine(), _length)))
, _local_ray(Ray3(ray.position, ray.direction / _length))
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
	auto length = _ray_cast.GetLength();
	auto scan_ray = _ray_cast.GetRay();
	if (_ray_cast.IsContacted())
	{
		auto contact_distance = _ray_cast.GetContactDistance();
		auto penetration_position = geom::Project(scan_ray, contact_distance);	
		gfx::Debug::AddLine(scan_ray.position, penetration_position, gfx::Debug::Color::Yellow());
		gfx::Debug::AddLine(penetration_position, geom::Project(scan_ray, length), gfx::Debug::Color::Red());
	}
	else
	{
		gfx::Debug::AddLine(scan_ray.position, geom::Project(scan_ray, length), gfx::Debug::Color::Green());
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
	scan_ray.direction += random_direction * _length * 0.05f;
	
	auto scan_length = geom::Length(scan_ray.direction);
	scan_ray.direction /= scan_length;
	
	// generate new ray
	_ray_cast.SetLength(scan_length);
	_ray_cast.SetRay(scan_ray);
}

core::locality::Roster & Sensor::GetTickRoster()
{
	return _entity.GetEngine().GetTickRoster();
}
