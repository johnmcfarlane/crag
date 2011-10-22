//
//  sim/Ball.cpp
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

#include "gfx/object/Ball.h"
#include "gfx/Renderer.h"


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
: _model(nullptr)
{
}

Ball::~Ball()
{
	gfx::RemoveObjectMessage message = { ref(_model) };
	gfx::Renderer::Daemon::SendMessage(message);
}

void Ball::Create(Ball & ball, PyObject & args)
{
	// construct ball
	new (& ball) Ball;
	
	// create message
	AddEntityMessage message = { ball, args };
	
	// send
	Simulation::Daemon::SendMessage(message);
}

bool Ball::Init(Simulation & simulation, PyObject & args)
{
	// Parse planet creation parameters
	Vector<double, 3> center;
	double radius;
	
	if (! PyArg_ParseTuple(& args, "dddd", & center.x, & center.y, & center.z, & radius))
	{
		return false;
	}
	
	// physics
	physics::Engine & physics_engine = simulation.GetPhysicsEngine();	
	physics::SphericalBody * body = new physics::SphericalBody(physics_engine, true, radius);
	body->SetPosition(center);
	body->SetDensity(ball_density);
	body->SetLinearDamping(ball_linear_damping);
	body->SetAngularDamping(ball_angular_damping);
	SetBody(body);
	
	{
		_model = new gfx::Ball(radius);
		gfx::AddObjectMessage message = { ref(_model) };
		gfx::Renderer::Daemon::SendMessage(message);
	}
	
	return true;
}

void Ball::Tick(Simulation & simulation)
{
	// Gravity
	Universe const & universe = simulation.GetUniverse();
	Body * body = GetBody();
	universe.ApplyGravity(* body);
}

void Ball::UpdateModels() const
{
	gfx::UpdateObjectMessage<gfx::Ball> message(ref(_model));

	physics::SphericalBody const * body = static_cast<physics::SphericalBody const *>(GetBody());
	if (body == nullptr)
	{
		return;
	}

	message._params._position = body->GetPosition();
	message._params._rotation = body->GetRotation();
	
	gfx::Renderer::Daemon::SendMessage(message);
}
