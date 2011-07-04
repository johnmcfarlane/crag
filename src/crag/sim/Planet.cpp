/*
 *  Planet.cpp
 *  Crag
 *
 *  Created by john on 4/14/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Planet.h"
#include "PlanetaryBody.h"
#include "PlanetShader.h"
#include "Simulation.h"

#include "MoonShader.h"

#include "form/Formation.h"
#include "form/scene/Mesh.h"
#include "form/FormationManager.h"

#include "script/MetaClass.h"


////////////////////////////////////////////////////////////////////////////////
// Planet script binding

DEFINE_SCRIPT_CLASS(sim, Planet)


//////////////////////////////////////////////////////////////////////
// Planet


sim::Planet::Planet()
: factory(nullptr)
, formation(nullptr)
, body(nullptr)
{
}

sim::Planet::~Planet()
{
	// unregister with formation manager
	form::RemoveFormationMessage message = { * formation };
	
	// TODO: This shouldn't have to be blocking. The probably cause
	// is that not only is formation being deleted here right away,
	// but also factory is deleted AND it keeps a reference to this planet. 
	// The solution is to put factory in formation and to keep planet out 
	// of shader.
	form::FormationManager::SendMessage(message, true);

	delete body;
	delete formation;
	delete factory;
}

void sim::Planet::Create(Planet & planet, PyObject & args)
{	
	// construct planet
	new (& planet) Planet;

	// create message
	AddEntityMessage message = { planet, args };
	
	// send
	Simulation::SendMessage(message, true);
}

bool sim::Planet::Init(PyObject & args)
{
	// Parse planet creation parameters
	sim::Vector3 center;
	int random_seed;
	int num_craters;
	if (! PyArg_ParseTuple(& args, "ddddii", & center.x, & center.y, & center.z, & radius_mean, & random_seed, & num_craters))
	{
		return false;
	}

	Assert(radius_mean > 0);
	radius_min = radius_mean;
	radius_max = radius_mean;

	// factory
	if (num_craters > 0)
	{
		factory = new MoonShaderFactory(* this, num_craters);
	}
	else 
	{
		factory = new PlanetShaderFactory(* this);
	}
		
	// formation
	formation = new form::Formation(* factory);
	formation->seed = random_seed;
	formation->SetPosition(center);
		
	// body
	{
		physics::Engine & physics_engine = sim::Simulation::Ref().GetPhysicsEngine();
		body = new PlanetaryBody(physics_engine, * formation, radius_mean);
		body->SetPosition(center);
	}
	
	// register with formation manager
	form::AddFormationMessage message = { * formation };
	form::FormationManager::SendMessage(message, false);
	
	return true;
}

void sim::Planet::Tick()
{
	body->SetRadius(radius_max);
}

void sim::Planet::GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const
{
	Vector3 const & center = body->GetPosition();
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

bool sim::Planet::GetRenderRange(Ray3 const & camera_ray, double * range, bool wireframe) const
{
	Scalar distance_squared = LengthSq(camera_ray.position - GetPosition());
	Scalar distance = Sqrt(distance_squared);
	
	// Is camera inside the planet?
	if (distance < radius_min)
	{
		range[0] = radius_min - distance;
		range[1] = distance + radius_max; 
		return true;
	}

	// Is camera outside the entire planet?
	if (distance > radius_max)
	{
		range[0] = distance - radius_max;
	}
	else 
	{
		// The camera is between the min and max range of planet heights 
		// so it could be right up close to stuff.
		range[0] = 0;
	}
	
	// Finally we need to calculate the furthest distance from the camera to the planet.
	
	// For wireframe mode, it's easy (and the same as when you're inside the planet.
	if (wireframe)
	{
		range[1] = distance + radius_max; 
		return true;
	}
	
	// Otherwise, the furthest you might ever be able too see of this planet
	// is a ray that skims the sphere of radius_min
	// and then hits the sphere of radius_max.
	// For some reason, those two distances appear to be very easy to calculate. 
	// (Famous last words.)
	Scalar a = Sqrt(Square(distance) - Square(radius_min));
	Scalar b = Sqrt(Square(radius_max) - Square(radius_min));
	range[1] = a + b;
	
	return true;
}

void sim::Planet::SampleRadius(Scalar r)
{
	if (radius_max < r)
	{
		radius_max = r;
	}
	else if (radius_min > r)
	{
		radius_min = r;
	}
}

form::Formation const & sim::Planet::GetFormation() const
{
	return ref(formation);
}

sim::Vector3 const & sim::Planet::GetPosition() const
{
	return body->GetPosition();
}
