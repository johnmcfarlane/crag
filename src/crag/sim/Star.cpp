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

#include "gfx/Scene.h"


sim::Star::Star(Scalar init_radius, Scalar init_year)
: Entity()
, light(Vector3f::Zero(), gfx::Color4f(.85f, .85f, .85f), 0, 0, 1, true)
, radius(init_radius)
, year(init_year)
{
	Simulation & sim = Simulation::Ref();
	gfx::Scene & scene = sim.GetScene();
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
	
	// create message
	Star * star = nullptr;
	AddStarMessage message = { orbital_radius, orbital_year };
	
	// send
	Simulation::SendMessage(message, star);

	return star;
}

void sim::Star::Tick()
{
	sys::TimeType t = sim::Simulation::Ref().GetTime();
	Scalar angle = static_cast<Scalar>(t * (2. * PI) / year) + 3.6;
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
