/*
 *  Star.cpp
 *  Crag
 *
 *  Created by John on 12/22/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Star.h"

#include "Simulation.h"
#include "Universe.h"


sim::Star::Star(SimulationPtr const & s, Scalar init_radius, Scalar init_year)
: Entity(s)
, light(Vector3f::Zero(), gfx::Color4f(.85f, .85f, .85f), 0, 0, 1, true)
, radius(init_radius)
, year(init_year)
{
	gfx::Scene & scene = s->GetScene();
	scene.AddLight(light);
}

sim::Star * sim::Star::Create(PyObject * args)
{
	// Parse planet creation parameters
	sim::Scalar orbital_radius;
	sim::Scalar orbital_year;
	if (! PyArg_ParseTuple(args, "dd", & orbital_radius, & orbital_year))
	{
		return nullptr;
	}
	
	// Create planet object.
	SimulationPtr s(Simulation::GetPtr());
	sim::Star * star = new sim::Star(s, orbital_radius, orbital_year);
	
	return star;
}

void sim::Star::Tick(Universe const & universe)
{
	Scalar angle = static_cast<Scalar>(universe.GetTime() * (2. * PI) / year) + 3.6;
	position = Vector3(- Sin(angle) * radius, - Cos(angle) * radius, static_cast<Scalar>(0));
	light.SetPosition(position);
}

sim::Scalar sim::Star::GetBoundingRadius() const
{
	return 0;	// really just a point light for now
}

sim::Vector3 const & sim::Star::GetPosition() const
{
	return position;
}
