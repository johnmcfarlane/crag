//
//  Planet.cpp
//  crag
//
//  Created by john on 4/14/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Planet.h"

#include "EntityFunctions.h"
#include "MoonShader.h"
#include "PlanetaryBody.h"
#include "PlanetShader.h"
#include "Simulation.h"

#include "form/Formation.h"
#include "form/FormationManager.h"

#include "gfx/Renderer.inl"

#include "gfx/object/Planet.h"

#include "script/MetaClass.h"


//#define RENDER_SEA


using namespace sim;


////////////////////////////////////////////////////////////////////////////////
// Planet script binding

DEFINE_SCRIPT_CLASS(sim, Planet)


//////////////////////////////////////////////////////////////////////
// sim::InitData<Planet> struct specialization

namespace sim
{
	template <>
	struct InitData<Planet>
	{
		Sphere3 sphere;
		int random_seed;
		int num_craters;
	};
}


//////////////////////////////////////////////////////////////////////
// sim::Planet member definitions


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
	gfx::Daemon::Call<gfx::Uid>(_transformation_uid, & gfx::Renderer::OnRemoveObject);
	
	// unregister with formation manager
	form::Daemon::Call<form::Formation *>(_formation, & form::FormationManager::OnRemoveFormation);
	_formation = nullptr;

	delete _body;
	_body = nullptr;
}

bool Planet::Create(Planet & planet, PyObject & args)
{	
	// Parse planet creation parameters
	InitData<Planet> init_data;
	if (! PyArg_ParseTuple(& args, "ddddii", & init_data.sphere.center.x, & init_data.sphere.center.y, & init_data.sphere.center.z, & init_data.sphere.radius, & init_data.random_seed, & init_data.num_craters))
	{
		return false;
	}

	// register with simulation
	sim::Daemon::Call(& planet, init_data, & sim::Simulation::OnNewEntity);
	return true;
}

void Planet::Init(Simulation & simulation, InitData<Planet> const & init_data)
{
	Assert(init_data.sphere.radius > 0);
	_radius_mean = init_data.sphere.radius;
	_radius_min = init_data.sphere.radius;
	_radius_max = init_data.sphere.radius;
	
	// allocations
	{
		Random random(init_data.random_seed);
		
		// factory
		form::Shader * shader;
		if (init_data.num_craters > 0)
		{
			int random_seed_shader = random.GetInt();
			shader = new MoonShader(random_seed_shader, init_data.num_craters, _radius_mean);
		}
		else 
		{
			shader = new PlanetShader();
		}
		
		// formation
		int random_seed_formation = random.GetInt();
		_formation = new form::Formation(random_seed_formation, * shader, init_data.sphere, GetUid());
		
		// body
		physics::Engine & physics_engine = simulation.GetPhysicsEngine();
		_body = new PlanetaryBody(physics_engine, ref(_formation), _radius_mean);
		_body->SetPosition(init_data.sphere.center);
	}

	// messages
	{
		// register with formation manager
		form::Daemon::Call(_formation, & form::FormationManager::OnAddFormation);
		
		// register with the renderer
#if defined(RENDER_SEA)
		float sea_level = _radius_mean;
#else
		float sea_level = 0;
#endif
		gfx::Planet * model = new gfx::Planet(sea_level);
		_model_uid = model->GetUid();
		_transformation_uid = AddModelWithTransform(* model);
	}
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
	{
		// update planet params
		Vector3 const & position = _body->GetPosition();
		gfx::BranchNode::UpdateParams params = 
		{
			Transformation(position, Transformation::Rotation::Identity(), _radius_mean)
		};
		
		gfx::Daemon::Call(_transformation_uid, params, & gfx::Renderer::OnUpdateObject<gfx::BranchNode>);
	}

	{
		// update planet params
		gfx::Planet::UpdateParams params = 
		{
			_radius_min,
			_radius_max
		};
		
		gfx::Daemon::Call(_model_uid, params, & gfx::Renderer::OnUpdateObject<gfx::Planet>);
	}
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
