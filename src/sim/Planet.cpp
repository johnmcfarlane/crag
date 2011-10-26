/*
 *  sim/Planet.cpp
 *  Crag
 *
 *  Created by john on 4/14/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Planet.h"

#include "MoonShader.h"
#include "PlanetaryBody.h"
#include "PlanetShader.h"
#include "Simulation.h"

#include "form/Formation.h"
#include "form/FormationManager.h"

#include "gfx/Renderer.h"

#include "gfx/object/Planet.h"

#include "script/MetaClass.h"


////////////////////////////////////////////////////////////////////////////////
// Planet script binding

DEFINE_SCRIPT_CLASS(sim, Planet)


//////////////////////////////////////////////////////////////////////
// Planet


sim::Planet::Planet()
: _formation(nullptr)
, _body(nullptr)
, _model(nullptr)
{
}

sim::Planet::~Planet()
{
	{
		// unregister with renderer
		gfx::RemoveObjectMessage message = { ref(_model) };
		gfx::Renderer::Daemon::SendMessage(message);
		_model = nullptr;
	}
	
	{
		// unregister with formation manager
		form::RemoveFormationMessage message = { ref(_formation) };
		form::FormationManager::Daemon::SendMessage(message);
		_formation = nullptr;
	}

	delete _body;
	_body = nullptr;
}

void sim::Planet::Create(Planet & planet, PyObject & args)
{	
	// construct planet
	new (& planet) Planet;

	// create message
	AddEntityMessage message = { planet, args };
	
	// send
	Simulation::Daemon::SendMessage(message);
}

bool sim::Planet::Init(Simulation & simulation, PyObject & args)
{
	// Parse planet creation parameters
	sim::Vector3 center;
	int random_seed;
	int num_craters;
	if (! PyArg_ParseTuple(& args, "ddddii", & center.x, & center.y, & center.z, & _radius_mean, & random_seed, & num_craters))
	{
		return false;
	}

	Assert(_radius_mean > 0);
	_radius_min = _radius_mean;
	_radius_max = _radius_mean;
	
	// allocations
	{
		// factory
		form::ShaderFactory * factory;
		if (num_craters > 0)
		{
			factory = new MoonShaderFactory(* this, num_craters);
		}
		else 
		{
			factory = new PlanetShaderFactory(* this);
		}
			
		// formation
		_formation = new form::Formation(* factory);
		_formation->seed = random_seed;
		_formation->SetPosition(center);
		
		// body
		physics::Engine & physics_engine = simulation.GetPhysicsEngine();
		_body = new PlanetaryBody(physics_engine, ref(_formation), _radius_mean);
		_body->SetPosition(center);

		// model
		_model = new gfx::Planet(center);
	}

	// messages
	{
		// register with formation manager
		{
			form::AddFormationMessage message = { ref(_formation) };
			form::FormationManager::Daemon::SendMessage(message);
		}
		
		// register with the renderer
		{
			gfx::AddObjectMessage message = { ref(_model) };
			gfx::Renderer::Daemon::SendMessage(message);
		}
	}
	
	return true;
}

void sim::Planet::Tick(Simulation & simulation)
{
	_body->SetRadius(_radius_max);
}

void sim::Planet::GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const
{
	Vector3 const & center = _body->GetPosition();
	Vector3 to_center = center - pos;
	sim::Scalar distance = Length(to_center);
	
	// Calculate the direction of the pull.
	Vector3 direction = to_center / distance;

	// Calculate the mass.
	sim::Scalar density = 1;
	sim::Scalar radius = GetRadiusMean();
	sim::Scalar volume = Cube(radius);
	sim::Scalar mass = volume * density;

	// Calculate the force. Actually, this isn't really the force;
	// It's the potential. Until we know what we're pulling we can't know the force.
	sim::Scalar force;
	if (distance < radius)
	{
		force = mass * distance / Cube(radius);
	}
	else
	{
		force = mass / Square(distance);
	}

	Vector3 contribution = direction * force;
	gravity += contribution;
}

void sim::Planet::UpdateModels() const
{
	gfx::UpdateObjectMessage<gfx::Planet> message(ref(_model));
	message._params._position = GetPosition();
	message._params._radius_min = _radius_min;
	message._params._radius_max = _radius_max;
	gfx::Renderer::Daemon::SendMessage(message);
}

void sim::Planet::SampleRadius(Scalar r)
{
	if (_radius_max < r)
	{
		_radius_max = r;
	}
	else if (_radius_min > r)
	{
		_radius_min = r;
	}
}

form::Formation const & sim::Planet::GetFormation() const
{
	return ref(_formation);
}

sim::Vector3 const & sim::Planet::GetPosition() const
{
	return _body->GetPosition();
}