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
: _body(nullptr)
, _model(nullptr)
{
}

Box::~Box()
{
	delete _body;
	
	{
		gfx::RemoveObjectMessage message = { ref(_model) };
		gfx::Renderer::Daemon::SendMessage(message);
	}
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
	_body = new physics::BoxBody(physics_engine, true, size);
	_body->SetPosition(center);
	_body->SetDensity(box_density);
	_body->SetLinearDamping(box_linear_damping);
	_body->SetAngularDamping(box_angular_damping);
	
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
	universe.ApplyGravity(* _body);
}

void Box::UpdateModels() const
{
	gfx::UpdateObjectMessage<gfx::Box> message(ref(_model));
	message._params._position = _body->GetPosition();
	_body->GetRotation(message._params._rotation);
	
	gfx::Renderer::Daemon::SendMessage(message);
}

Vector3 const & Box::GetPosition() const
{
	return _body->GetPosition();
}
