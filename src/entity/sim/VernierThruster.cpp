//
//  entity/sim/VernierThruster.cpp
//  crag
//
//  Created by John McFarlane on 2014-03-18.
//	Copyright 2014 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "VernierThruster.h"

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "physics/RayCast.h"

#include "gfx/axes.h"

#include "core/ConfigEntry.h"

using namespace sim;

namespace
{
	CONFIG_DEFINE(thrust_ground_effect_max_distance, 50.f);
	CONFIG_DEFINE(thrust_ground_effect_factor, 50.f);
}

////////////////////////////////////////////////////////////////////////////////
// sim::Thruster member definitions

VernierThruster::VernierThruster(Entity & entity, Ray3 const & ray)
: Thruster(entity, ray, false, 0.f)
, _ray_cast(new physics::RayCast(entity.GetEngine().GetPhysicsEngine(), thrust_ground_effect_max_distance))
{
	CRAG_VERIFY(* this);
}

void VernierThruster::Tick()
{
	CRAG_VERIFY(* this);

	auto location = GetEntity().GetLocation();
	if (! location)
	{
		DEBUG_BREAK("thruster attached to eneity with no location");
		return;
	}

	auto & body = core::StaticCast<physics::Body>(* location);

	auto const & local_ray = GetRay();
	auto global_ray = body.GetTransformation().Transform(local_ray);
	global_ray.direction /= - geom::Length(global_ray.direction);
	
	auto thrust_factor = 1.f;
	
	thrust_factor += [&] ()
	{
		auto const & result = _ray_cast->GetResult();
		if (! result)
		{
			// ray cast isn't colliding with anything
			return 0.f;
		}
		
		auto ground_distance = result.GetDistance();
		auto inverse_square = thrust_ground_effect_factor / Squared(ground_distance);
		auto cusion = inverse_square;
		CRAG_VERIFY_OP(cusion, >, 0.f);
		
		return cusion;
	} ();
	
	SetThrustFactor(thrust_factor);
	
	// update ray cast
	_ray_cast->SetRay(global_ray);
	
	Thruster::Tick();
}
