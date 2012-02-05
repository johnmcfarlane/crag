//
//  Box.cpp
//  crag
//
//  Created by John McFarlane on 9/5/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Box.h"

#include "EntityFunctions.h"
#include "Simulation.h"

#include "physics/BoxBody.h"

#include "script/MetaClass.h"

#include "gfx/object/Box.h"
#include "gfx/Renderer.inl"

#include "geom/Transformation.h"

#include "core/Random.h"


namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// Config values
	
	CONFIG_DEFINE (box_density, double, 1);
	
	CONFIG_DEFINE (box_linear_damping, double, 0.005f);
	CONFIG_DEFINE (box_angular_damping, double, 0.005f);
}


////////////////////////////////////////////////////////////////////////////////
// sim::Box script binding

DEFINE_SCRIPT_CLASS(sim, Box)


////////////////////////////////////////////////////////////////////////////////
// sim::InitData<Box> struct specialization

namespace sim
{
	template <>
	struct InitData<Box>
	{
		Vector3 center;
		Vector3 size;
	};
}


////////////////////////////////////////////////////////////////////////////////
// sim::Box script binding

using namespace sim;


Box::Box()
{
}

Box::~Box()
{
	gfx::Daemon::Call<gfx::Uid>(_gfx_uid, & gfx::Renderer::OnRemoveObject);
}

bool Box::Create(Box & box, PyObject & args)
{
	// Parse planet creation parameters
	InitData<Box> init_data;
	if (! PyArg_ParseTuple(& args, "dddddd", & init_data.center.x, & init_data.center.y, & init_data.center.z, & init_data.size.x, & init_data.size.y, & init_data.size.z))
	{
		return false;
	}

	// send
	Daemon::Call< Box *, InitData<Box> >(& box, init_data, & Simulation::OnNewEntity);
	return true;
}

void Box::Init(Simulation & simulation, InitData<Box> const & init_data)
{
	InitPhysics(simulation, init_data.center, init_data.size);
	InitGraphics();
}

void Box::InitPhysics(Simulation & simulation, Vector3 center, Vector3 size)
{
	// physics
	physics::Engine & physics_engine = simulation.GetPhysicsEngine();	
	physics::BoxBody * body = new physics::BoxBody(physics_engine, true, size);
	body->SetPosition(center);
	body->SetDensity(box_density);
	body->SetLinearDamping(box_linear_damping);
	body->SetAngularDamping(box_angular_damping);
	SetBody(body);
}

void Box::InitGraphics()
{
	gfx::Color4f color(Random::sequence.GetUnitInclusive<float>(), 
		Random::sequence.GetUnitInclusive<float>(), 
		Random::sequence.GetUnitInclusive<float>(), 
		Random::sequence.GetUnitInclusive<float>());
	gfx::Object * box = new gfx::Box(color);
	_gfx_uid = AddModelWithTransform(* box);
}

void Box::UpdateModels() const
{
	physics::BoxBody const * body = static_cast<physics::BoxBody const *>(GetBody());
	if (body == nullptr)
	{
		return;
	}
	
	gfx::BranchNode::UpdateParams params = 
	{
		Transformation(body->GetPosition(), body->GetRotation(), body->GetDimensions())
	};
	
	gfx::Daemon::Call(_gfx_uid, params, & gfx::Renderer::OnUpdateObject<gfx::BranchNode>);
}
