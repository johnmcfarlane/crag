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

#include "EntityFunctions.h"
#include "Simulation.h"

#include "physics/SphericalBody.h"

#include "script/MetaClass.h"

#include "gfx/object/Ball.h"
#include "gfx/object/BranchNode.h"
#include "gfx/Renderer.inl"


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
// sim::Ball member definitions

using namespace sim;


Ball::~Ball()
{
	gfx::Daemon::Call<gfx::Uid>(_gfx_uid, & gfx::Renderer::OnRemoveObject);
}

void Ball::Create(Ball & ball, PyObject & args)
{
	// construct ball
	new (& ball) Ball;
	
	// send
	Daemon::Call<Entity *>(& ball, & args, & Simulation::OnAddEntity);
}

bool Ball::Init(Simulation & simulation, PyObject & args)
{
	// Parse planet creation parameters
	Sphere3 sphere;
	
	if (! PyArg_ParseTuple(& args, "dddd", & sphere.center.x, & sphere.center.y, & sphere.center.z, & sphere.radius))
	{
		return false;
	}
	
	InitPhysics(simulation, sphere);

	InitGraphics(sphere);
	
	return true;
}

void Ball::InitPhysics(Simulation & simulation, Sphere3 const & sphere)
{
	// physics
	physics::Engine & physics_engine = simulation.GetPhysicsEngine();	
	physics::SphericalBody * body = new physics::SphericalBody(physics_engine, true, sphere.radius);
	body->SetPosition(sphere.center);
	body->SetDensity(ball_density);
	body->SetLinearDamping(ball_linear_damping);
	body->SetAngularDamping(ball_angular_damping);
	SetBody(body);
}

void Ball::InitGraphics(Sphere3 const & sphere)
{
	gfx::Object * ball = new gfx::Ball();
	_gfx_uid = AddModelWithTransform(* ball);
}

void Ball::UpdateModels() const
{
	physics::SphericalBody const * body = static_cast<physics::SphericalBody const *>(GetBody());
	if (body == nullptr)
	{
		return;
	}

	Scalar radius = body->GetRadius();
	Vector3 scale(radius, radius, radius);
	gfx::BranchNode::UpdateParams params = 
	{
		Transformation(body->GetPosition(), body->GetRotation(), scale)
	};
	
	gfx::Daemon::Call(_gfx_uid, params, & gfx::Renderer::OnUpdateObject<gfx::BranchNode>);
}
