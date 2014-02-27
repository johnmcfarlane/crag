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
	CONFIG_DEFINE (gravitational_force, float, 0.000000001f);

	// Given a position and a mass at that position, returns a 
	// reasonable approximation of the weight vector at that position.
	Vector3 Weight(Engine & engine, Vector3 pos, Scalar mass)
	{
		Vector3 force = Vector3::Zero();
		
		engine.ForEachObject([& pos, & force] (Entity const & entity) {
			auto location = entity.GetLocation();
			if (location)
			{
				force += location->GetGravitationalForce(pos);
			}
		});
		
		return force * Scalar(mass) * Scalar(gravitational_force);
	}

	
	void ApplyGravity(Engine & engine, core::Time delta, physics::Body & body) 
	{
		Vector3 const & position = body.GetTranslation();
		Scalar mass = body.GetMass();
		if (mass <= 0)
		{
			return;
		}
		
		Vector3 gravitational_force_per_second = Weight(engine, position, mass);
		Vector3 gravity = gravitational_force_per_second / Scalar(delta);
		
		body.AddForce(gravity);
	}
	
}

void sim::ApplyGravity(Engine & engine, core::Time delta)
{
	engine.ForEachObject([&] (Entity & entity) {
		physics::Body * body = entity.GetBody();
		if (body == nullptr || ! body->ObeysGravity())
		{
			return;
		}
		
		::ApplyGravity(engine, delta, * body);
	});
}

