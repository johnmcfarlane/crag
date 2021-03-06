//
//  gravity.cpp
//  crag
//
//  Created by John McFarlane on 11/6/11.
//  Copyright 2009 - 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "gravity.h"

#include "Entity.h"

#include "physics/Body.h"

#include "core/ConfigEntry.h"


using namespace sim;

namespace
{
	CONFIG_DEFINE(gravitational_constant, 0.0000000025f);

	// Given a position and a mass at that position, returns a 
	// reasonable approximation of the weight vector at that position.
	Vector3 Weight(Engine & engine, Vector3 pos, Scalar mass)
	{
		Vector3 force = GetGravitationalForce(engine, pos);

		return force * Scalar(mass) * Scalar(gravitational_constant);
	}

	
	void ApplyGravity(Engine & engine, core::Time delta, physics::Location & location) 
	{
		Vector3 const & position = location.GetTranslation();
		auto & body = core::StaticCast<physics::Body>(location);
		Scalar mass = body.GetMass();
		if (mass <= 0)
		{
			return;
		}
		
		Vector3 gravitational_force_per_second = Weight(engine, position, mass);
		Vector3 gravity = gravitational_force_per_second / Scalar(delta);
		
		body.SetGravitationalForce(gravity);
	}
	
}

Vector3 sim::GetGravitationalForce(Engine & engine, Vector3 position)
{
	Vector3 force = Vector3::Zero();

	engine.ForEachObject([&] (Entity const & entity) {
		auto location = entity.GetLocation();
		if (location)
		{
			force += location->GetGravitationalAttraction(position);
		}
	});

	return force;
}

void sim::ApplyGravity(Engine & engine, core::Time delta)
{
	engine.ForEachObject([&] (Entity & entity) {
		auto const & location = entity.GetLocation();
		if (! location || ! location->ObeysGravity())
		{
			return;
		}
		
		::ApplyGravity(engine, delta, * location);
	});
}

Vector3 sim::GetUp(Vector3 const & gravitational_force)
{
	auto force_magnitude_sq = geom::MagnitudeSq(gravitational_force);
	if (! force_magnitude_sq)
	{
		return Vector3::Zero();
	}
	
	return gravitational_force * (-1.f / std::sqrt(force_magnitude_sq));
}
