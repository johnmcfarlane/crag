//
//  gravity.cpp
//  crag
//
//  Created by John McFarlane on 11/6/11.
//  Copyright 2009 - 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Entity.h"
#include "gravity.h"

#include "physics/Body.h"

#include "core/ConfigEntry.h"


using namespace sim;


namespace
{
	CONFIG_DEFINE (gravitational_force, float, 0.0000000025f);

	// Given a position and a mass at that position, returns a 
	// reasonable approximation of the weight vector at that position.
	Vector3 Weight(Entity::List const & entities, Vector3 const & pos, Scalar mass)
	{
		Vector3 force = Vector3::Zero();
		
		for (Entity::List::const_iterator it = entities.begin(); it != entities.end(); ++ it) 
		{
			Entity const & e = * it;
			e.GetGravitationalForce(pos, force);
		}
		
		return force * static_cast<Scalar>(mass) * static_cast<Scalar>(gravitational_force);
	}

	
	void ApplyGravity(Entity::List const & entities, Time delta, physics::Body & body) 
	{
		Vector3 const & position = body.GetPosition();
		Scalar mass = body.GetMass();
		
		Vector3 gravitational_force_per_second = Weight(entities, position, mass);
		Vector3 gravity = gravitational_force_per_second / delta;
		
		body.AddForce(gravity);
	}
	
}

void sim::ApplyGravity(Entity::List & entities, Time delta)
{
	for (Entity::List::iterator i = entities.begin(), end = entities.end(); i != end; ++ i)
	{
		Entity & entity = * i;
		
		physics::Body * body = entity.GetBody();
		if (body == nullptr)
		{
			continue;
		}
		
		::ApplyGravity(entities, delta, * body);
	}
}

