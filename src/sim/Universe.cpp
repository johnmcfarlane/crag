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
CONFIG_DEFINE_MEMBER (Universe, apply_gravity, bool, true);


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
	apply_gravity = ! apply_gravity;
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
void Universe::Tick(Simulation & simulation, sys::TimeType target_frame_seconds)
{
	time += target_frame_seconds;
	
	for (EntityVector::const_iterator it = entities.begin(); it != entities.end(); ++ it)
	{
		Entity & e = * * it;
		e.Tick(simulation);
	}
}

// Given a position and a mass at that position, returns a 
// reasonable approximation of the weight vector at that position.
Vector3 Universe::Weight(Vector3 const & pos, Scalar mass) const
{
	if (apply_gravity) 
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
	Vector3 gravity = gravitational_force_per_second / Simulation::target_frame_seconds;
	
	body.AddForce(gravity);
}

void Universe::ApplyGravity(physics::Body & body, Vector3 const & center_of_mass) const
{
	Vector3 const & position = body.GetPosition();
	Scalar mass = body.GetMass();
	
	Vector3 gravitational_force_per_second = Weight(position, mass);
	Vector3 gravity = gravitational_force_per_second / Simulation::target_frame_seconds;
	
	body.AddRelForceAtRelPos(gravity, center_of_mass);
}

void Universe::Purge()
{
	for (EntityVector::const_iterator it = entities.begin(); it != entities.end(); ++ it) 
	{
		Entity & e = * * it;
		physics::Body const * body = e.GetBody();
		if (body == nullptr)
		{
			continue;
		}

		Vector3 position = body->GetPosition();
		if (position == position)
		{
			continue;
		}

		std::cerr << "purging entity with bad position" << std::endl;
		delete body;
		e.SetBody(nullptr);
	}
}

void Universe::UpdateModels() const
{
	for (EntityVector::const_iterator it = entities.begin(); it != entities.end(); ++ it) 
	{
		Entity & e = * * it;
		e.UpdateModels();
	}
}
