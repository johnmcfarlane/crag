/*
 *  Universe.cpp
 *  Crag
 *
 *  Created by john on 4/28/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Universe.h"
#include "Entity.h"

#include "physics/Singleton.h"


//////////////////////////////////////////////////////////////////////
// local definitions

namespace sim
{
	namespace 
	{
#if defined(NDEBUG) && defined(PROFILE)
		bool gravity = false;
#else
		CONFIG_DEFINE (gravity, bool, true);
#endif
		
		CONFIG_DEFINE (gravitational_force, float, 0.0000000025f);
	}
}


//////////////////////////////////////////////////////////////////////
// Universe definitions

CONFIG_DEFINE_MEMBER (sim::Universe, target_frame_seconds, double, 1.f / 60.f);


sim::Universe::Universe()
: time(0)
{
	Assert(entities.size() == 0);
}

sim::Universe::~Universe()
{
	Assert(entities.empty());
}

sys::TimeType sim::Universe::GetTime() const
{
	return time;
}

void sim::Universe::ToggleGravity()
{
	gravity = ! gravity;
}

void sim::Universe::AddEntity(Entity & entity)
{
	Assert(std::find(entities.begin(), entities.end(), & entity) == entities.end());
	entities.push_back(& entity);
}

void sim::Universe::RemoveEntity(Entity & entity)
{
	EntityVector::iterator i = std::find(entities.begin(), entities.end(), & entity);
	Assert(i != entities.end());
	entities.erase(i);
	
	Assert(std::find(entities.begin(), entities.end(), & entity) == entities.end());
}

// Perform a step in the simulation. 
void sim::Universe::Tick(bool physics)
{
	time += target_frame_seconds;
	
	if (physics)
	{
		physics::Singleton & physics_singleton = physics::Singleton::Get();
		physics_singleton.Tick(target_frame_seconds);
	}
	
	for (EntityVector::const_iterator it = entities.begin(); it != entities.end(); ++ it)
	{
		Entity & e = * * it;
		e.Tick(* this);
	}
}

// Given a position and a mass at that position, returns a 
// reasonable approximation of the weight vector at that position.
sim::Vector3 sim::Universe::Weight(Vector3 const & pos, Scalar mass) const
{
	if (gravity) 
	{
		sim::Vector3 force = sim::Vector3::Zero();
		
		for (EntityVector::const_iterator it = entities.begin(); it != entities.end(); ++ it) 
		{
			Entity & e = * * it;
			e.GetGravitationalForce(pos, force);
		}
		
		return force * static_cast<sim::Scalar>(mass) * static_cast<sim::Scalar>(gravitational_force);
	}
	else 
	{
		return sim::Vector3::Zero();
	}
}

