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
	CONFIG_DEFINE (observer_radius, double, .5);
	CONFIG_DEFINE (observer_density, double, 1);
	
	CONFIG_DEFINE (observer_speed_factor, double, 631);

	CONFIG_DEFINE (observer_linear_damping, double, 0.025f);
	CONFIG_DEFINE (observer_angular_damping, double, 0.05f);
	CONFIG_DEFINE (observer_velocity_impulse, float, 0.002f);
	CONFIG_DEFINE (observer_torque_impulse, double, .0025f);

	CONFIG_DEFINE (observer_light_color, geom::Vector3f, geom::Vector3f(0.6f, 0.8f, 1.0f) * 1.f);
}


////////////////////////////////////////////////////////////////////////////////
// Observer	member definitions

Observer::Observer(Entity::Init const & init, Vector3 const & center)
: Entity(init)
, speed_factor(observer_speed_factor)
{
	physics::Engine & physics_engine = init.engine.GetPhysicsEngine();
	physics::SphericalBody * body = new physics::SphericalBody(physics_engine, true, observer_radius);
	SetSpeed(1);
	
	body->SetDensity(observer_density);
	body->SetLinearDamping(observer_linear_damping);
	body->SetAngularDamping(observer_angular_damping);
	body->SetPosition(center);

	SetBody(body);
	
	impulses[0] = impulses[1] = Vector3::Zero();
	
#if defined(OBSERVER_LIGHT)
	// register light with the renderer
	gfx::Light * light = new gfx::Light(observer_light_color);
	_light_uid = AddModelWithTransform(* light);
#endif
}

Observer::~Observer()
{
#if defined(OBSERVER_LIGHT)
	_model.Destroy();
#endif

	observer_speed_factor = static_cast<double>(speed_factor);
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
	
	Scalar inv_t = 1.f / sim_tick_duration;

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
	speed_factor = static_cast<double>(pow(pow(10., .4), static_cast<double>((speed << 1) + 1)));
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
	Matrix33 const & rotation = body->GetRotation();

	// Give renderer the new camera position.
	{
		Transformation transformation (position, rotation);
		gfx::Daemon::Call([transformation] (gfx::Engine & engine) {
			engine.OnSetCamera(transformation);
		});
		
		Ray3 camera_ray = axes::GetCameraRay(transformation);
		sim::Daemon::Call([camera_ray] (sim::Engine & engine) {
			engine.SetCamera(camera_ray);
		});
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
