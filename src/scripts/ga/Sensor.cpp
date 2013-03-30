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
	auto & scanner = _ray;

	auto start = Transform(scanner.position, _entity);
	auto end = Transform(geom::Project(scanner, 1.0f), _entity);
	gfx::Debug::ColorPair cp(gfx::Debug::Color::Red(), gfx::Debug::Color(0,0,0,0));

	gfx::Debug::AddLine(start, end, cp);
}

core::locality::Roster & Sensor::GetTickRoster()
{
	return _entity.GetEngine().GetTickRoster();
}
