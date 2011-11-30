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

#include "gfx/object/Light.h"
#include "gfx/Renderer.inl"

#include "script/MetaClass.h"


////////////////////////////////////////////////////////////////////////////////
// sim::Star script binding

DEFINE_SCRIPT_CLASS(sim, Star)


////////////////////////////////////////////////////////////////////////////////
// sim::Star member definitions

sim::Star::Star()
: Entity()
, radius(-1)
, year(-1)
{
}

sim::Star::~Star()
{
	gfx::Daemon::Call<gfx::Uid>(_light_uid, & gfx::Renderer::OnRemoveObject);
}

void sim::Star::Create(Star & star, PyObject & args)
{
	// construct star
	new (& star) Star;
	
	// send
	Daemon::Call<Entity *, PyObject *>(& star, & args, & Simulation::OnAddEntity);
}

bool sim::Star::Init(Simulation & simulation, PyObject & args)
{
	// Parse planet creation parameters
	if (! PyArg_ParseTuple(& args, "dd", & radius, & year))
	{
		return false;
	}

	// initialize light
	gfx::Light * light = new gfx::Light(Vector3f::Zero(), gfx::Color4f(.85f, .85f, .85f), 0, 0, 1, true);
	_light_uid = light->GetUid();
	gfx::Daemon::Call<gfx::Object *, gfx::Uid>(light, gfx::Uid::null, & gfx::Renderer::OnAddObject);
	
	return true;
}

void sim::Star::Tick(Simulation & simulation)
{
	Time t = simulation.GetTime();
	Scalar angle = static_cast<Scalar>(t * (2. * PI) / year) + 3.6;
	position = Vector3(- Sin(angle) * radius, - Cos(angle) * radius, static_cast<Scalar>(0));
}

void sim::Star::UpdateModels() const
{
	gfx::Light::UpdateParams params = 
	{
		GetPosition()
	};
	
	gfx::Daemon::Call(_light_uid, params, & gfx::Renderer::OnUpdateObject<gfx::Light>);
}

sim::Scalar sim::Star::GetBoundingRadius() const
{
	return 0;	// really just a point light for now
}

sim::Vector3 const & sim::Star::GetPosition() const
{
	return position;
}
