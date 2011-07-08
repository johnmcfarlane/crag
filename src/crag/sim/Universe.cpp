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

#include "Simulation.h"
#include "Entity.h"

#include "physics/Body.h"


using namespace sim;

//////////////////////////////////////////////////////////////////////
// Universe definitions

CONFIG_DEFINE_MEMBER (Universe, gravitational_force, float, 0.0000000025f);
CONFIG_DEFINE_MEMBER (Universe, gravity, bool, true);


Universe::Universe()
: time(0)
{
}

Universe::~Universe()
{
	Assert(entities.empty());
}

sys::TimeType Universe::GetTime() const
{
	return time;
}

void Universe::ToggleGravity()
{
	gravity = ! gravity;
}

void Universe::AddEntity(Entity & entity)
{
	Assert(std::find(entities.begin(), entities.end(), & entity) == entities.end());
	entities.push_back(& entity);
}

void Universe::RemoveEntity(Entity & entity)
{
	EntityVector::iterator i = std::find(entities.begin(), entities.end(), & entity);
	Assert(i != entities.end());
	entities.erase(i);
	
	Assert(std::find(entities.begin(), entities.end(), & entity) == entities.end());
}

// Perform a step in the simulation. 
void Universe::Tick(sys::TimeType target_frame_seconds)
{
	time += target_frame_seconds;
	
	for (EntityVector::const_iterator it = entities.begin(); it != entities.end(); ++ it)
	{
		Entity & e = * * it;
		e.Tick();
	}
}

// Given a position and a mass at that position, returns a 
// reasonable approximation of the weight vector at that position.
Vector3 Universe::Weight(Vector3 const & pos, Scalar mass) const
{
	if (gravity) 
	{
		Vector3 force = Vector3::Zero();
		
		for (EntityVector::const_iterator it = entities.begin(); it != entities.end(); ++ it) 
		{
			Entity & e = * * it;
			e.GetGravitationalForce(pos, force);
		}
		
		return force * static_cast<Scalar>(mass) * static_cast<Scalar>(gravitational_force);
	}
	else 
	{
		return Vector3::Zero();
	}
}

void Universe::ApplyGravity(physics::Body & body) const
{
	Vector3 const & position = body.GetPosition();
	Scalar mass = body.GetMass();
	
	Vector3 gravitational_force_per_second = Weight(position, mass);
	Vector3 gravitational_force = gravitational_force_per_second / Simulation::target_frame_seconds;
	
	body.AddForce(gravitational_force);
}

void Universe::ApplyGravity(physics::Body & body, Vector3 const & center_of_mass) const
{
	Vector3 const & position = body.GetPosition();
	Scalar mass = body.GetMass();
	
	Vector3 gravitational_force_per_second = Weight(position, mass);
	Vector3 gravitational_force = gravitational_force_per_second / Simulation::target_frame_seconds;
	
	body.AddRelForceAtRelPos(gravitational_force, center_of_mass);
}
