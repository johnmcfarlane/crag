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

#include "gfx/Renderer.inl"

#include "gfx/object/Planet.h"

#include "script/MetaClass.h"


using namespace sim;


////////////////////////////////////////////////////////////////////////////////
// Planet script binding

DEFINE_SCRIPT_CLASS(sim, Planet)


//////////////////////////////////////////////////////////////////////
// Planet


Planet::Planet()
: _formation(nullptr)
, _body(nullptr)
, _radius_mean(0)
, _radius_min(0)
, _radius_max(0)
{
}

Planet::~Planet()
{
	// unregister with renderer
	gfx::Daemon::Call<gfx::Uid>(_model_uid, & gfx::Renderer::OnRemoveObject);
	
	// unregister with formation manager
	form::Daemon::Call<form::Formation *>(_formation, & form::FormationManager::OnRemoveFormation);
	_formation = nullptr;

	delete _body;
	_body = nullptr;
}

void Planet::Create(Planet & planet, PyObject & args)
{	
	// construct planet
	new (& planet) Planet;

	// register with simulation
	sim::Daemon::Call<Entity *>(& planet, & args, & sim::Simulation::OnAddEntity);
}

bool Planet::Init(Simulation & simulation, PyObject & args)
{
	// Parse planet creation parameters
	Sphere3 sphere;
	int random_seed;
	int num_craters;
	if (! PyArg_ParseTuple(& args, "ddddii", & sphere.center.x, & sphere.center.y, & sphere.center.z, & sphere.radius, & random_seed, & num_craters))
	{
		return false;
	}

	Assert(sphere.radius > 0);
	_radius_mean = sphere.radius;
	_radius_min = sphere.radius;
	_radius_max = sphere.radius;
	
	// allocations
	{
		Random random(random_seed);
		
		// factory
		form::Shader * shader;
		if (num_craters > 0)
		{
			int random_seed_shader = random.GetInt();
			shader = new MoonShader(random_seed_shader, num_craters, _radius_mean);
		}
		else 
		{
			shader = new PlanetShader();
		}
		
		// formation
		int random_seed_formation = random.GetInt();
		_formation = new form::Formation(random_seed_formation, * shader, sphere, GetUid());
		
		// body
		physics::Engine & physics_engine = simulation.GetPhysicsEngine();
		_body = new PlanetaryBody(physics_engine, ref(_formation), _radius_mean);
		_body->SetPosition(sphere.center);
	}

	// messages
	{
		// register with formation manager
		form::Daemon::Call(_formation, & form::FormationManager::OnAddFormation);
		
		// register with the renderer
		_model_uid = gfx::Uid::Create();
		gfx::Planet * model = new gfx::Planet(sphere.center);
		gfx::Daemon::Call<gfx::Uid, gfx::Object *, gfx::Uid>(_model_uid, model, gfx::Uid::null, & gfx::Renderer::OnAddObject);
	}
	
	return true;
}

void Planet::Tick(Simulation & simulation)
{
	_body->SetRadius(_radius_max);
}

void Planet::GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const
{
	Vector3 const & center = _body->GetPosition();
	Vector3 to_center = center - pos;
	Scalar distance = Length(to_center);
	
	// Calculate the direction of the pull.
	Vector3 direction = to_center / distance;

	// Calculate the mass.
	Scalar density = 1;
	Scalar radius = GetRadiusMean();
	Scalar volume = Cube(radius);
	Scalar mass = volume * density;

	// Calculate the force. Actually, this isn't really the force;
	// It's the potential. Until we know what we're pulling we can't know the force.
	Scalar force;
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

void Planet::UpdateModels() const
{
	gfx::Planet::UpdateParams params = 
	{
		GetPosition(),
		_radius_min,
		_radius_max
	};
	
	gfx::Daemon::Call(_model_uid, params, & gfx::Renderer::OnUpdateObject<gfx::Planet>);
}

Scalar Planet::GetRadiusMean() const 
{ 
	return _radius_mean; 
}

Scalar Planet::GetRadiusMin() const 
{ 
	return _radius_min; 
}

Scalar Planet::GetRadiusMax() const 
{ 
	return _radius_max; 
}

void Planet::SetRadiusMinMax(Scalar radius_min, Scalar radius_max)
{
	Assert(radius_min <= _radius_min);
	_radius_min = radius_min;

	Assert(radius_max >= _radius_max);
	_radius_max = radius_max;
}

form::Formation const & Planet::GetFormation() const
{
	return ref(_formation);
}

Vector3 const & Planet::GetPosition() const
{
	return _body->GetPosition();
}
