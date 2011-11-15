//
//  Vehicle.cpp
//  crag
//
//  Created by John McFarlane on 11/9/11.
//	Copyright 2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Vehicle.h"

#include "Simulation.h"

#include "physics/Body.h"

#include "script/MetaClass.h"

using namespace sim;


////////////////////////////////////////////////////////////////////////////////
// sim::Vehicle script binding

namespace
{
	PyObject * vehicle_add_rotation(PyObject * self, PyObject * args)
	{
		sim::Vehicle::Rocket rocket;
		if (! PyArg_ParseTuple(args, "ddddddi", 
							   & rocket.position.x, & rocket.position.y, & rocket.position.z, 
							   & rocket.direction.x, & rocket.direction.y, & rocket.direction.z,
							   & rocket.key))
		{
			Py_RETURN_NONE;
		}
		
		Vehicle & vehicle = Vehicle::GetRef(self);
		vehicle.AddRocket(rocket);
		
		Py_RETURN_NONE;
	}
}

DEFINE_SCRIPT_CLASS_BEGIN(sim, Vehicle)
	SCRIPT_CLASS_METHOD("add_rocket", vehicle_add_rotation, "Add a rocket (x,y,z,p,q,r,key)")
DEFINE_SCRIPT_CLASS_END


////////////////////////////////////////////////////////////////////////////////
// sim::Vehicle member functions


void Vehicle::AddRocket(Rocket const & rocket)
{
	_rockets.push_back(rocket);
}

void Vehicle::Create(Vehicle & vehicle, PyObject & args)
{
	// construct vehicle
	new (& vehicle) Vehicle;
	
	// send
	Daemon::Call<Entity *>(& vehicle, & args, & Simulation::OnAddEntity);
}

bool Vehicle::Init(Simulation & simulation, PyObject & args)
{
	return super::Init(simulation, args);
}

void Vehicle::UpdateModels() const
{
	return super::UpdateModels();
}

void Vehicle::Tick(Simulation & simulation) override
{
	physics::Body * body = GetBody();
	if (body == nullptr)
	{
		Assert(false);
		return;
	}
	
	for (RocketVector::const_iterator i = _rockets.begin(), end = _rockets.end(); i != end; ++ i)
	{
		Rocket const & rocket = * i;
		ApplyForce(rocket, * body);
	}
}

void Vehicle::ApplyForce(Rocket const & rocket, Body & body)
{
	if (sys::IsKeyDown(rocket.key))
	{
		body.AddRelForceAtRelPos(rocket.direction, rocket.position);
	}
}
