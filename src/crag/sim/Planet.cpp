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

#include "core/Random.h"


//////////////////////////////////////////////////////////////////////
// Planet

sim::Planet::Planet(sim::Vector3 const & init_pos, Scalar init_radius_mean, int init_seed, int num_craters)
: radius_mean(init_radius_mean)
, radius_min(init_radius_mean)
, radius_max(init_radius_mean)
{
	Assert(radius_mean > 0);

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
	Random rnd(init_seed);
	formation->seed = rnd.GetInt();
	formation->SetPosition(init_pos);
	
	// body
	{
		physics::Engine & physics_engine = sim::Simulation::Ref().GetPhysicsEngine();
		body = new PlanetaryBody(physics_engine, * formation, init_radius_mean);
		body->SetPosition(init_pos);
	}

	// register with formation manager
	form::AddFormationMessage message = { * formation };
	form::FormationManager::SendMessage(message);
}

sim::Planet::~Planet()
{
	// unregister with formation manager
	form::RemoveFormationMessage message = { * formation };
	form::FormationManager::SendMessage(message);

	delete body;
	delete formation;
	delete factory;
}

sim::Planet * sim::Planet::Create(PyObject * args)
{
	// Parse planet creation parameters
	sim::Vector3 center;
	sim::Scalar radius;
	int random_seed;
	int num_craters;
	if (! PyArg_ParseTuple(args, "ddddii", & center.x, & center.y, & center.z, & radius, & random_seed, & num_craters))
	{
		return nullptr;
	}
	
	// create message
	Planet * planet = reinterpret_cast<Planet *>(new char [sizeof(Planet)]);
	AddPlanetMessage message = { center, radius, random_seed, num_craters, * planet };
	
	// send
	Simulation::SendMessage(message, true);

	return planet;
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
