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

#include "script/MetaClass.h"


////////////////////////////////////////////////////////////////////////////////
// sim::Star script binding

DEFINE_SCRIPT_CLASS(sim, Star)


////////////////////////////////////////////////////////////////////////////////
// sim::Star member definitions

sim::Star::Star()
: Entity()
, light(Vector3f::Zero(), gfx::Color4f(.85f, .85f, .85f), 0, 0, 1, true)
, radius(-1)
, year(-1)
{
	Simulation & sim = Simulation::Ref();
	gfx::Scene & scene = sim.GetScene();
	scene.AddLight(light);
}

void sim::Star::Create(Star & star, PyObject & args)
{
	// construct star
	new (& star) Star;
	
	// create message
	AddEntityMessage message = { star, args };
	
	// send
	Simulation::SendMessage(message);
}

bool sim::Star::Init(PyObject & args)
{
	// Parse planet creation parameters
	if (! PyArg_ParseTuple(& args, "dd", & radius, & year))
	{
		return false;
	}
	
	return true;
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
