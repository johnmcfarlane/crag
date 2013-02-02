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
#include "Engine.h"

#include "form/Formation.h"
#include "form/Engine.h"


#include "gfx/object/Planet.h"


//#define RENDER_SEA


using namespace sim;


//////////////////////////////////////////////////////////////////////
// sim::Planet member definitions

DEFINE_POOL_ALLOCATOR(Planet, 3);

Planet::Planet(Entity::Init const & init, Sphere3 sphere, int random_seed, int num_craters)
: Entity(init)
, _formation(nullptr)
, _radius_mean(sphere.radius)
, _radius_min(sphere.radius)
, _radius_max(sphere.radius)
{
	ASSERT(sphere.radius > 0);
	
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
		geom::abs::Sphere3 formation_sphere = geom::RelToAbs(sphere, GetEngine().GetOrigin());
		_formation = new form::Formation(random_seed_formation, * shader, formation_sphere, * this);
		GetEngine().AddFormation(* _formation);
		
		// body
		physics::Engine & physics_engine = init.engine.GetPhysicsEngine();
		auto body = new PlanetaryBody(physics_engine, ref(_formation), physics::Scalar(_radius_mean));
		body->SetPosition(geom::Cast<physics::Scalar>(sphere.center));
		SetLocation(body);
	}
	
	// messages
	{
		// register with the renderer
#if defined(RENDER_SEA)
		gfx::Scalar sea_level = _radius_mean;
#else
		gfx::Scalar sea_level = 0;
#endif
		
		_branch_node.Create(gfx::Transformation::Matrix44::Identity());
		_model.Create(gfx::Scalar(sea_level));
		auto model = _model;
		auto branch_node = _branch_node;
		gfx::Daemon::Call([model, branch_node] (gfx::Engine & engine) {
			engine.OnSetParent(model.GetUid(), branch_node.GetUid());
		});
		UpdateModels();
	}
}

Planet::~Planet()
{
	// unregister with renderer
	_branch_node.Destroy();
	
	// unregister with formation manager
	GetEngine().RemoveFormation(* _formation);
	_formation = nullptr;
}

void Planet::Tick()
{
	// TODO: This could be moved into SetRadiusMinMax.
	PlanetaryBody & body = static_cast<PlanetaryBody &>(* GetBody());
	body.SetRadius(physics::Scalar(_radius_max));
}

void Planet::GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const
{
	Vector3 const & center = GetBody()->GetPosition();
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
		Vector3 const & position = GetBody()->GetPosition();
		gfx::Transformation transformation(position, Transformation::Matrix33::Identity(), physics::Scalar(_radius_mean));
		_branch_node.Call([transformation] (gfx::BranchNode & node) {
			node.SetTransformation(transformation);
		});
	}

	{
		// update planet params
		gfx::Planet::UpdateParams params = 
		{
			_radius_min,
			_radius_max
		};
		_model.Call([params] (gfx::Planet & planet) {
			planet.Update(params);
		});
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

void Planet::SetRadiusMinMax(Scalar const & radius_min, Scalar const & radius_max)
{
	ASSERT(radius_min <= _radius_min);
	_radius_min = radius_min;

	ASSERT(radius_max >= _radius_max);
	_radius_max = radius_max;
}

form::Formation const & Planet::GetFormation() const
{
	return ref(_formation);
}
