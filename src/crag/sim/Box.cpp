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
	gfx::RemoveObjectMessage message = { ref(_model) };
	gfx::Renderer::Daemon::SendMessage(message);
}

void Box::Create(Box & box, PyObject & args)
{
	// construct box
	new (& box) Box;
	
	// create message
	AddEntityMessage message = { box, args };
	
	// send
	Simulation::Daemon::SendMessage(message);
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
	
	{
		_model = new gfx::Box(size);
		gfx::AddObjectMessage message = { ref(_model) };
		gfx::Renderer::Daemon::SendMessage(message);
	}
	
	return true;
}

void Box::Tick(Simulation & simulation)
{
	// Gravity
	Universe const & universe = simulation.GetUniverse();
	universe.ApplyGravity(* GetBody());
}

void Box::UpdateModels() const
{
	gfx::UpdateObjectMessage<gfx::Box> message(ref(_model));

	physics::BoxBody const * body = static_cast<physics::BoxBody const *>(GetBody());
	message._params.transformation = Transformation(body->GetPosition(), body->GetRotation(), body->GetDimensions());
	
	gfx::Renderer::Daemon::SendMessage(message);
}
