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

#include "gfx/object/Light.h"
#include "gfx/Renderer.h"

#include "script/MetaClass.h"


////////////////////////////////////////////////////////////////////////////////
// sim::Star script binding

DEFINE_SCRIPT_CLASS(sim, Star)


////////////////////////////////////////////////////////////////////////////////
// sim::Star member definitions

sim::Star::Star()
: Entity()
, light(nullptr)
, radius(-1)
, year(-1)
{
}

sim::Star::~Star()
{
	gfx::RemoveObjectMessage message = { ref(light) };
	gfx::Renderer::Daemon::SendMessage(message);
}

void sim::Star::Create(Star & star, PyObject & args)
{
	// construct star
	new (& star) Star;
	
	// create message
	AddEntityMessage message = { star, args };
	
	// send
	Simulation::Daemon::SendMessage(message);
}

bool sim::Star::Init(Simulation & simulation, PyObject & args)
{
	// Parse planet creation parameters
	if (! PyArg_ParseTuple(& args, "dd", & radius, & year))
	{
		return false;
	}

	// initialize light
	light = new gfx::Light(Vector3f::Zero(), gfx::Color4f(.85f, .85f, .85f), 0, 0, 1, true);
	
	// pass to the renderer
	gfx::AddObjectMessage message = { ref(light) };
	gfx::Renderer::Daemon::SendMessage(message);
	
	return true;
}

void sim::Star::Tick(Simulation & simulation)
{
	sys::TimeType t = simulation.GetTime();
	Scalar angle = static_cast<Scalar>(t * (2. * PI) / year) + 3.6;
	position = Vector3(- Sin(angle) * radius, - Cos(angle) * radius, static_cast<Scalar>(0));
}

void sim::Star::UpdateModels() const
{
	gfx::UpdateObjectMessage<gfx::Light> message(ref(light));
	
	message._params = GetPosition();
	
	gfx::Renderer::Daemon::SendMessage(message);
}

sim::Scalar sim::Star::GetBoundingRadius() const
{
	return 0;	// really just a point light for now
}

sim::Vector3 const & sim::Star::GetPosition() const
{
	return position;
}
