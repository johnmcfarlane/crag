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

#include "Simulation.h"
#include "Universe.h"

#include "physics/BoxBody.h"

#include "script/MetaClass.h"

#include "gfx/object/Box.h"
#include "gfx/Renderer.h"

#include "geom/Transformation.h"


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

DEFINE_SCRIPT_CLASS(sim, Box);


////////////////////////////////////////////////////////////////////////////////
// sim::Box script binding

using namespace sim;


Box::Box()
: _model(nullptr)
{
}

Box::~Box()
{
	gfx::Daemon::Call<gfx::Object *>(_model, & gfx::Renderer::OnRemoveObject);
}

void Box::Create(Box & box, PyObject & args)
{
	// construct box
	new (& box) Box;
	
	// send
	Daemon::Call<Entity *, PyObject *>(& box, & args, & Simulation::OnAddEntity);
}

bool Box::Init(Simulation & simulation, PyObject & args)
{
	// Parse planet creation parameters
	Vector<double, 3> center;
	Vector<double, 3> size;
	
	if (! PyArg_ParseTuple(& args, "dddddd", & center.x, & center.y, & center.z, & size.x, & size.y, & size.z))
	{
		return false;
	}
	
	// physics
	physics::Engine & physics_engine = simulation.GetPhysicsEngine();	
	physics::BoxBody * body = new physics::BoxBody(physics_engine, true, size);
	body->SetPosition(center);
	body->SetDensity(box_density);
	body->SetLinearDamping(box_linear_damping);
	body->SetAngularDamping(box_angular_damping);
	SetBody(body);
	
	_model = new gfx::Box(size);
	gfx::Daemon::Call<gfx::Object *>(_model, & gfx::Renderer::OnAddObject);
	
	return true;
}

void Box::Tick(Simulation & simulation)
{
	Body * body = GetBody();
	if (body == nullptr)
	{
		return;
	}

	// Gravity
	// TODO: Body is now a 'component' this can doesn't have to happen here.
	Universe const & universe = simulation.GetUniverse();
	universe.ApplyGravity(* body);
}

void Box::UpdateModels() const
{
	physics::BoxBody const * body = static_cast<physics::BoxBody const *>(GetBody());
	if (body == nullptr)
	{
		return;
	}

	gfx::Box::UpdateParams params = 
	{
		Transformation(body->GetPosition(), body->GetRotation(), body->GetDimensions())
	};

	gfx::Daemon::Call(_model, params, & gfx::Renderer::OnUpdateObject<gfx::Box>);
}
