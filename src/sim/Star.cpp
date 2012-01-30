//
//  Star.cpp
//  crag
//
//  Created by John on 12/22/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Star.h"

#include "EntityFunctions.h"
#include "Simulation.h"

#include "gfx/object/Light.h"
#include "gfx/Renderer.inl"

#include "script/MetaClass.h"


////////////////////////////////////////////////////////////////////////////////
// sim::Star script binding

DEFINE_SCRIPT_CLASS(sim, Star)


////////////////////////////////////////////////////////////////////////////////
// sim::InitData<Star> struct specialization

namespace sim
{
	template <>
	struct InitData <Star>
	{
		Scalar radius;
		Scalar year;
	};
}


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

bool sim::Star::Create(Star & star, PyObject & args)
{
	// Parse planet creation parameters
	InitData<Star> init_data;
	if (! PyArg_ParseTuple(& args, "dd", & init_data.radius, & init_data.year))
	{
		return false;
	}

	// send
	Daemon::Call(& star, init_data, & Simulation::OnNewEntity);
	return true;
}

void sim::Star::Init(Simulation & simulation, InitData<Star> const & init_data)
{
	radius = init_data.radius;
	year = init_data.year;
	
	// initialize light
	gfx::Light * light = new gfx::Light(gfx::Color4f(1.f,.95f,.9f) * 7500000000000000.f);
	_light_uid = AddModelWithTransform(* light);
}

void sim::Star::Tick(Simulation & simulation)
{
	Time t = simulation.GetTime();
	Scalar angle = static_cast<Scalar>(t * (2. * PI) / year) + 3.6;
	position = Vector3(- sin(angle) * radius, - cos(angle) * radius, static_cast<Scalar>(0));
}

void sim::Star::UpdateModels() const
{
	gfx::BranchNode::UpdateParams params = 
	{
		Transformation(position)
	};
	
	gfx::Daemon::Call(_light_uid, params, & gfx::Renderer::OnUpdateObject<gfx::BranchNode>);
}

sim::Scalar sim::Star::GetBoundingRadius() const
{
	return 0;	// really just a point light for now
}
