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
		
		sys::TimeType time = 0;
		
		typedef std::vector<sim::Entity *> EntityList;
		EntityList entities;

	}
}


//////////////////////////////////////////////////////////////////////
// Universe definitions

CONFIG_DEFINE_MEMBER (sim::Universe, target_frame_seconds, double, 1.f / 60.f);


sim::Universe::Universe()
{
	time = 0;
	
	Assert(entities.size() == 0);
}

sim::Universe::~Universe()
{
	for (EntityList::iterator it = entities.begin(); it != entities.end(); ++ it)
	{
		Entity * e = * it;
		delete e;
	}
	entities.clear();
}

sys::TimeType sim::Universe::GetTime() const
{
	return time;
}

void sim::Universe::ToggleGravity()
{
	gravity = ! gravity;
}

// TODO: Better to have a RemoveEntity and just assert that array is empty in d'tor.
void sim::Universe::AddEntity(Entity & entity)
{
	Assert(std::find(entities.begin(), entities.end(), & entity) == entities.end());
	entities.push_back(& entity);
}

// Perform a step in the simulation. 
void sim::Universe::Tick()
{
	time += target_frame_seconds;
	
	physics::Singleton & physics_singleton = physics::Singleton::Get();
	physics_singleton.Tick(target_frame_seconds);
	
	for (EntityList::const_iterator it = entities.begin(); it != entities.end(); ++ it)
	{
		Entity & e = * * it;
		e.Tick();
	}
}

// Given a position and a mass at that position, returns a 
// reasonable approximation of the weight vector at that position.
sim::Vector3 sim::Universe::Weight(Vector3 const & pos, Scalar mass) const
{
	if (gravity) 
	{
		sim::Vector3 force = sim::Vector3::Zero();
		
		for (EntityList::const_iterator it = entities.begin(); it != entities.end(); ++ it) 
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

// Given a camera position/direction, conservatively estimates 
// the minimum and maximum distances at which rendering occurs.
// TODO: Long-term, this function needs to be replaced with 
// something that gives and near and far plane value instead. 
void sim::Universe::GetRenderRange(Ray3 const & camera_ray, double & range_min, double & range_max, bool wireframe) const
{
	for (EntityList::const_iterator it = entities.begin(); it != entities.end(); ++ it) 
	{
		Entity & e = * * it;
		double entity_range[2];
		if (e.GetRenderRange(camera_ray, entity_range, wireframe))
		{
			if (entity_range[0] < range_min)
			{
				range_min = entity_range[0];
			}
			
			if (entity_range[1] > range_max)
			{
				range_max = entity_range[1];
			}
		}
	}
}
