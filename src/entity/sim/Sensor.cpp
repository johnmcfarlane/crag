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

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "physics/Engine.h"

#include <geom/utils.h>

#include "core/Random.h"
#include "core/RosterObjectDefine.h"

using namespace sim;

namespace
{
	Ray3 Transform(Ray3 const & local, Entity const & entity)
	{
		auto location = entity.GetLocation();
		auto const & transformation = location->GetTransformation();

		auto global = transformation.Transform(local);

		return global;
	}
}

////////////////////////////////////////////////////////////////////////////////
// sim::Sensor member definitions

#if defined(CRAG_DEBUG)
CRAG_ROSTER_OBJECT_DEFINE(
	Sensor,
	1000,
	Pool::Call<& Sensor::DebugDraw>(Engine::GetDrawRoster()),
	Pool::Call<& Sensor::GenerateScanRay>(physics::Engine::GetPreTickRoster()),
	Pool::Call<& Sensor::SendReading>(physics::Engine::GetPostTickRoster()))
#else
CRAG_ROSTER_OBJECT_DEFINE(
		Sensor,
		1000,
		Pool::Call<&Sensor::GenerateScanRay>(physics::Engine::GetPreTickRoster()),
		Pool::Call<&Sensor::SendReading>(physics::Engine::GetPostTickRoster()))
#endif

Sensor::Sensor(Entity & entity, Ray3 const & ray, Scalar length, Scalar variance)
: _entity(entity)
, _length(length)
, _variance(variance)
, _ray_cast(new physics::RayCast(entity.GetEngine().GetPhysicsEngine(), length))
, _local_ray(ray)
{
	GenerateScanRay();
	
	auto & location = * entity.GetLocation();
	auto & body = core::StaticCast<physics::Body>(location);
	_ray_cast->SetIsCollidable(body, false);
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Sensor, self)
	CRAG_ROSTER_OBJECT_VERIFY(self);
	CRAG_VERIFY(self._local_ray.position);
	CRAG_VERIFY_UNIT(self._local_ray.direction, .0001f);
	CRAG_VERIFY_EQUAL(self._ray_cast->GetLength(), self._length);
CRAG_VERIFY_INVARIANTS_DEFINE_END

// update global sensor ray position
void Sensor::GenerateScanRay() noexcept
{
	CRAG_VERIFY(* this);
	
	Ray3 scan_ray = GetGlobalRay();
	CRAG_VERIFY_UNIT(scan_ray.direction, .0001f);

	scan_ray.direction *= _length;
	
	auto random_direction = geom::RandomVector<Scalar>(Random::sequence);
	scan_ray.direction += random_direction * _length * _variance;
	
	auto scan_length = geom::Magnitude(scan_ray.direction);
	scan_ray.direction /= scan_length;
	
	// generate new ray
	_ray_cast->SetRay(scan_ray);
}

void Sensor::SendReading() noexcept
{
	TransmitSignal(CalcReading());
}

#if defined(CRAG_DEBUG)
void Sensor::DebugDraw() noexcept
{
	_ray_cast->DebugDraw();
}
#endif

Scalar Sensor::CalcReading() const
{
	const auto & result = _ray_cast->GetResult();
	if (! result)
	{
		return 0.f;
	}

	auto contact_distance = result.GetDistance();
	CRAG_VERIFY_OP (contact_distance, >=, 0);
	CRAG_VERIFY_OP (contact_distance, <=, _length);

	auto ratio = 1.f - contact_distance / _length;
	ASSERT(ratio >= 0);
	ASSERT(ratio <= 1.f);

	return ratio;
}

Ray3 Sensor::GetGlobalRay() const
{
	return Transform(_local_ray, _entity);
}
