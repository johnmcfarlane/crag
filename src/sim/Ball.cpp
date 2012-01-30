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
// sim::Ball script binding

DEFINE_SCRIPT_CLASS(sim, Ball)


////////////////////////////////////////////////////////////////////////////////
// sim::InitData<Ball> struct specialization

namespace sim
{
	template <>
	struct InitData<Ball>
	{
		Sphere3 sphere;
	};
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
	gfx::Daemon::Call<gfx::Uid>(_gfx_uid, & gfx::Renderer::OnRemoveObject);
}

bool Ball::Create(Ball & ball, PyObject & args)
{
	// Parse planet creation parameters
	InitData<Ball> init_data;
	if (! PyArg_ParseTuple(& args, "dddd", & init_data.sphere.center.x, & init_data.sphere.center.y, & init_data.sphere.center.z, & init_data.sphere.radius))
	{
		return false;
	}
	
	Daemon::Call<Ball *, InitData<Ball>>(& ball, init_data, & Simulation::OnNewEntity);
	return true;
}

void Ball::Init(Simulation & simulation, InitData<Ball> const & init_data)
{
	InitPhysics(simulation, init_data.sphere);
	
	InitGraphics(init_data.sphere);
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
	gfx::Color4f color = GetColor();
	gfx::Object * ball = new gfx::Ball(color);
	_gfx_uid = AddModelWithTransform(* ball);
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
	gfx::BranchNode::UpdateParams params = 
	{
		Transformation(body->GetPosition(), body->GetRotation(), scale)
	};
	
	gfx::Daemon::Call(_gfx_uid, params, & gfx::Renderer::OnUpdateObject<gfx::BranchNode>);
}

gfx::Uid Ball::GetGfxUid() const
{
	Assert(_gfx_uid != gfx::Uid::null);
	return _gfx_uid;
}
