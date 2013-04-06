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

#include "physics/Location.h"

#include "gfx/Debug.h"

#include "core/Random.h"
#include "core/Roster.h"

using namespace sim;

namespace
{
	// given a position which is relative to entity, returns simulation-relative position
	Vector3 Transform(Vector3 local, Entity const & entity)
	{
		auto location = entity.GetLocation();
		return location->Transform(local);
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
, _ray(ray)
{
	auto & roster = GetTickRoster();
	roster.AddOrdering(& Sensor::Tick, & AnimatController::Tick);
	roster.AddCommand(* this, & Sensor::Tick);
}

Sensor::~Sensor()
{
	auto & roster = GetTickRoster();
	roster.RemoveCommand(* this, & Sensor::Tick);
}

void Sensor::Tick()
{
	Ray3 scan_ray = GenerateScanRay();
	
	gfx::Debug::ColorPair cp(gfx::Debug::Color::White(), gfx::Debug::Color(0,0,0,0));
	gfx::Debug::AddLine(scan_ray.position, geom::Project(scan_ray, 1.f), cp);
}

Ray3 Sensor::GenerateScanRay() const
{
	auto length = geom::Length(_ray.direction);
	auto random_direction = GetRandomDirection(Random::sequence);
	auto local_end = geom::Project(_ray, 1.f) + random_direction * length * 0.75f;
	auto end = Transform(local_end, _entity);

	Ray3 scan_ray;
	scan_ray.position = Transform(_ray.position, _entity);
	scan_ray.direction = end - scan_ray.position;
	
	return scan_ray;
}

core::locality::Roster & Sensor::GetTickRoster()
{
	return _entity.GetEngine().GetTickRoster();
}
