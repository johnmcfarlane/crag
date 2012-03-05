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

#include "gfx/object/Ball.h"
#include "gfx/object/BranchNode.h"

#include "core/Random.h"


namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// Config values
	
	CONFIG_DEFINE (ball_density, double, 1);
	
	CONFIG_DEFINE (ball_linear_damping, double, 0.005f);
	CONFIG_DEFINE (ball_angular_damping, double, 0.005f);
}


////////////////////////////////////////////////////////////////////////////////
// sim::Ball member definitions

using namespace sim;


// seems to be required by MetaClass::InitObject
Ball::Ball()
{
}

Ball::~Ball()
{
	_model.Destroy();
}

void Ball::Init(Simulation & simulation, Sphere3 const & sphere)
{
	InitPhysics(simulation, sphere);
	
	InitGraphics(sphere);
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
	// Create ball model.
	gfx::Color4f color(GetColor());

	gfx::Daemon::Call(true, & gfx::Renderer::OnSetReady);
	_model = AddModelWithTransform<gfx::Ball>(color);
	UpdateModels();
	gfx::Daemon::Call(false, & gfx::Renderer::OnSetReady);
}

gfx::Color4f Ball::GetColor() const
{
	gfx::Color4f color(Random::sequence.GetUnitInclusive<float>(), 
		Random::sequence.GetUnitInclusive<float>(), 
		Random::sequence.GetUnitInclusive<float>(), 
		Random::sequence.GetUnitInclusive<float>());
	return color;
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
	gfx::Transformation transformation(body->GetPosition(), body->GetRotation(), scale);

	_model.Call<gfx::Transformation>(& gfx::BranchNode::SetTransformation, transformation);
}

gfx::BranchNodeHandle const & Ball::GetModel() const
{
	return _model;
}
