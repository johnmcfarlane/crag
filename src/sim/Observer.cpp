//
//  Observer.cpp
//  crag
//
//  Created by john on 5/13/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
// 

#include "pch.h"

#include "axes.h"
#include "EntityFunctions.h"
#include "Observer.h"
#include "Engine.h"

#include "physics/SphericalBody.h"

#include "geom/Transformation.h"

#include "core/app.h"
#include "core/ConfigEntry.h"

#include "gfx/object/Light.h"

#include "geom/MatrixOps.h"


using namespace sim;


CONFIG_DECLARE (sim_tick_duration, core::Time);

namespace 
{
	CONFIG_DEFINE (observer_radius, Scalar, .5);
	CONFIG_DEFINE (observer_density, Scalar, 1);
	
	CONFIG_DEFINE (observer_speed_factor, Scalar, 631);

	CONFIG_DEFINE (observer_linear_damping, physics::Scalar, 0.025f);
	CONFIG_DEFINE (observer_angular_damping, physics::Scalar, 0.05f);
	CONFIG_DEFINE (observer_velocity_impulse, physics::Scalar, 0.002f);
	CONFIG_DEFINE (observer_torque_impulse, physics::Scalar, .0025f);

	CONFIG_DEFINE (observer_light_color, geom::Vector3f, geom::Vector3f(0.6f, 0.8f, 1.0f) * 1.f);
}


////////////////////////////////////////////////////////////////////////////////
// Observer	member definitions

Observer::Observer(Entity::Init const & init, axes::VectorAbs const & center)
: Entity(init)
, speed_factor(observer_speed_factor)
{
	Engine& sim_engine = GetEngine();
	physics::Engine & physics_engine = sim_engine.GetPhysicsEngine();
	physics::SphericalBody * body = new physics::SphericalBody(physics_engine, true, observer_radius);
	
	body->SetDensity(observer_density);
	body->SetLinearDamping(observer_linear_damping);
	body->SetAngularDamping(observer_angular_damping);

	axes::VectorRel center_rel = axes::AbsToRel(center, sim_engine.GetOrigin());
	body->SetPosition(center_rel);

	SetBody(body);
	
	impulses[0] = impulses[1] = Vector3::Zero();
	
#if defined(OBSERVER_LIGHT)
	// register light with the renderer
	gfx::Light * light = new gfx::Light(observer_light_color);
	_light_uid = AddModelWithTransform(* light);
#endif

	SetSpeed(1);
}

Observer::~Observer()
{
#if defined(OBSERVER_LIGHT)
	_model.Destroy();
#endif

	observer_speed_factor = speed_factor;
}

void Observer::AddRotation(Vector3 const & angles)
{
	impulses[1] += angles;
}

void Observer::UpdateInput(Controller::Impulse const & impulse)
{
	Scalar velocity_impulse = observer_velocity_impulse * speed_factor;
	Vector3 const impulse_factors[2] = 
	{
		Vector3(velocity_impulse, velocity_impulse, velocity_impulse),
		Vector3(observer_torque_impulse, observer_torque_impulse, observer_torque_impulse),
	};
	
	Scalar inv_t = Scalar(1. / sim_tick_duration);

	for (int d = 0; d < 2; ++ d)
	{
		for (int axis = 0; axis < 3; ++ axis)
		{
			Scalar factor = impulse.factors[d][axis];

			impulses[d][axis] += impulse_factors[d][axis] * factor * inv_t;
		}
	}
}

void Observer::SetSpeed(int const & speed)
{
	speed_factor = std::pow(std::pow(10.f, .4f), (speed << 1) + 1);
}

void Observer::Tick(sim::Engine & simulation_engine)
{
	// Camera input.
	UserInput ui;
	Controller::Impulse impulse = ui.GetImpulse();
	UpdateInput(impulse);
	
	ApplyImpulse();
}

void Observer::UpdateModels() const
{
	Body const * body = GetBody();
	if (body == nullptr)
	{
		return;
	}

	Vector3 const & position = body->GetPosition();
	Matrix33 rotation = body->GetRotation();
	Transformation transformation (position, rotation);

	// Give renderer the new camera position.
	gfx::Daemon::Call([transformation] (gfx::Engine & engine) {
		engine.OnSetCamera(transformation);
	});
	
	// Give simulation the new camera position.
	{
		Ray3 camera_ray = axes::GetCameraRay(transformation);
		auto& engine = GetEngine();
		engine.SetCamera(camera_ray);
	}

#if defined(OBSERVER_LIGHT)
	// Give renderer the new light position.
	{
		gfx::BranchNode::UpdateParams params = 
		{
			Transformation(position)
		};
		gfx::Daemon::Call(_light_uid, params, & gfx::Engine::OnUpdateObject<gfx::BranchNode>);
	}
#endif
}

void Observer::ApplyImpulse()
{
	Body * body = GetBody();

	body->AddRelForce(impulses [0]);
	impulses[0] = Vector3::Zero();
	
	body->AddRelTorque(impulses [1]);
	impulses[1] = Vector3::Zero();
}
