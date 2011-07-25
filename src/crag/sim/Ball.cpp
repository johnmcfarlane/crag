//
//  Ball.cpp
//  crag
//
//  Created by John McFarlane on 6/23/11.
//	Copyright 2009-2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Ball.h"

#include "Simulation.h"
#include "Universe.h"

#include "physics/SphericalBody.h"

#include "script/MetaClass.h"

#include "gfx/Ball.h"
#include "gfx/Renderer.h"
//#include "gfx/Scene.h"
//#include "gfx/Sphere.h"


namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// Config values
	
	CONFIG_DEFINE (ball_density, double, 1);

	CONFIG_DEFINE (ball_linear_damping, double, 0.005f);
	CONFIG_DEFINE (ball_angular_damping, double, 0.005f);
}


////////////////////////////////////////////////////////////////////////////////
// sim::Ball script binding

DEFINE_SCRIPT_CLASS(sim, Ball);


////////////////////////////////////////////////////////////////////////////////
// sim::Ball script binding

using namespace sim;


Ball::Ball()
: _body(nullptr)
, _model(nullptr)
{
}

Ball::~Ball()
{
	delete _body;
	
	{
		gfx::RemoveObjectMessage message = { ref(_model) };
		gfx::Renderer::SendMessage(message);
	}
}

void Ball::Create(Ball & ball, PyObject & args)
{
	// construct ball
	new (& ball) Ball;
	
	// create message
	AddEntityMessage message = { ball, args };
	
	// send
	Simulation::SendMessage(message);
}

bool Ball::Init(PyObject & args)
{
	// Parse planet creation parameters
	Vector<double, 3> center;
	double radius;
	
	if (! PyArg_ParseTuple(& args, "dddd", & center.x, & center.y, & center.z, & radius))
	{
		return false;
	}
	
	// physics
	physics::Engine & physics_engine = Simulation::Ref().GetPhysicsEngine();	
	_body = new physics::SphericalBody(physics_engine, true, radius);
	_body->SetPosition(center);
	_body->SetDensity(ball_density);
	_body->SetLinearDamping(ball_linear_damping);
	_body->SetAngularDamping(ball_angular_damping);
	
	{
		_model = new gfx::Ball(radius);
		gfx::AddObjectMessage message = { ref(_model) };
		gfx::Renderer::SendMessage(message);
	}
	
	return true;
}

void Ball::Tick()
{
	// Gravity
	Universe const & universe = Simulation::Ref().GetUniverse();
	universe.ApplyGravity(* _body);
}

void Ball::UpdateModels() const
{
	gfx::UpdateObjectMessage<gfx::Ball> message(ref(_model));
	message._updated._position = _body->GetPosition();
	_body->GetRotation(message._updated._rotation);
	
	gfx::Renderer::SendMessage(message);
}

Vector3 const & Ball::GetPosition() const
{
	return _body->GetPosition();
}
