/*
 *  Observer.cpp
 *  Crag
 *
 *  Created by john on 5/13/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */ 

#include "pch.h"

#include "axes.h"
#include "Observer.h"
#include "Simulation.h"
#include "Universe.h"

#include "physics/SphericalBody.h"

#include "geom/VectorOps.h"
#include "geom/MatrixOps.h"
#include "geom/Vector4.h"
#include "core/ConfigEntry.h"

#include "gfx/Debug.h"
#include "gfx/Scene.h"


namespace 
{
	CONFIG_DEFINE (observer_radius, double, .5);
	CONFIG_DEFINE (observer_density, double, 1);
	
	CONFIG_DEFINE (observer_speed_factor, double, 631);
	CONFIG_DEFINE (observer_gravity_center, sim::Vector3, sim::Vector3(0., 0., -.25));

	CONFIG_DEFINE (observer_linear_damping, double, 0.025f);
	CONFIG_DEFINE (observer_angular_damping, double, 0.05f);
	CONFIG_DEFINE (observer_velocity_impulse, float, 0.002f);
	CONFIG_DEFINE (observer_torque_impulse, double, .0025f);

	CONFIG_DEFINE (observer_light_color, gfx::Color4f, gfx::Color4f(0.8f, 0.8f, 1.0f));
	CONFIG_DEFINE (observer_light_attenuation_a, float, 0.00000001f);
	CONFIG_DEFINE (observer_light_attenuation_b, float, 0.000f);
	CONFIG_DEFINE (observer_light_attenuation_c, float, 4.000f);
}


sim::Observer::Observer(Vector3 const & init_pos)
: Entity()
, sphere(Simulation::Ref().GetPhysicsEngine(), true, observer_radius)
, speed(0)
, speed_factor(observer_speed_factor)
, light(Vector3::Zero(), observer_light_color, observer_light_attenuation_a, observer_light_attenuation_b, observer_light_attenuation_c)
{
	SetSpeedFactor(1);
	
	sphere.SetDensity(observer_density);
	sphere.SetLinearDamping(observer_linear_damping);
	sphere.SetAngularDamping(observer_angular_damping);

	impulses[0] = impulses[1] = Vector3::Zero();
	
	SetPosition(init_pos);

	gfx::Scene & scene = Simulation::Ref().GetScene();
	scene.AddLight(light);
}

sim::Observer::~Observer()
{
	observer_speed_factor = static_cast<double>(speed_factor);
}

sim::Observer * sim::Observer::Create(PyObject * args)
{
	// Parse planet creation parameters
	Vector<double, 3> center;
	if (! PyArg_ParseTuple(args, "ddd", &center.x, &center.y, &center.z))
	{
		return nullptr;
	}

	// create message
	Observer * observer = reinterpret_cast<Observer *>(new char [sizeof(Observer)]);
	AddObserverMessage message = { center, * observer };
	
	// send
	Simulation::SendMessage(message, true);
	
	return observer;
}

void sim::Observer::AddRotation(Vector3 const & angles)
{
	impulses[1] += angles;
}

void sim::Observer::UpdateInput(Controller::Impulse const & impulse)
{
	Scalar velocity_impulse = observer_velocity_impulse * speed_factor;
	Vector3 const impulse_factors[2] = 
	{
		Vector3(velocity_impulse, velocity_impulse, velocity_impulse),
		Vector3(observer_torque_impulse, observer_torque_impulse, observer_torque_impulse),
	};
	
	Scalar inv_t = 1.f / Simulation::target_frame_seconds;

	for (int d = 0; d < 2; ++ d)
	{
		for (int axis = 0; axis < 3; ++ axis)
		{
			Scalar factor = impulse.factors[d][axis];

			impulses[d][axis] += impulse_factors[d][axis] * factor * inv_t;
		}
	}
}

void sim::Observer::SetSpeedFactor(int _speed_factor)
{
	// TODO: Specify a range in config.
	speed_factor = static_cast<double>(Power(Power(10., .4), static_cast<double>((_speed_factor << 1) + 1)));
}

void sim::Observer::Tick()
{
	// Camera input.
	if (sys::HasFocus()) 
	{
		UserInput ui;
		Controller::Impulse impulse = ui.GetImpulse();
		UpdateInput(impulse);
	}
	
	ApplyImpulse();

	// Gravity
	Vector3 const & position = GetPosition();
	Scalar mass = sphere.GetMass();
	
	Universe & universe = Simulation::Ref().GetUniverse();
	
	Vector3 gravitational_force_per_second = universe.Weight(position, mass);
	Vector3 gravitational_force = gravitational_force_per_second / Simulation::target_frame_seconds;

	Vector3 scaled_observer_gravity_center = observer_gravity_center * sphere.GetRadius();
	sphere.AddRelForceAtRelPos(gravitational_force, scaled_observer_gravity_center);
	
	// Light
	light.SetPosition(position);

	// Set simulation camera.
	Vector3 pos = sphere.GetPosition();
	Matrix4 rot;
	sphere.GetRotation(rot);
	
	SetCameraMessage message = { position, rot };
	Simulation::SendMessage(message, false);
}

sim::Vector3 const & sim::Observer::GetPosition() const
{
	return sphere.GetPosition();
}

void sim::Observer::SetPosition(sim::Vector3 const & pos) 
{
	light.SetPosition(pos);
	sphere.SetPosition(pos);
}

void sim::Observer::ApplyImpulse()
{
	sphere.AddRelForce(impulses [0]);
	impulses[0] = Vector3::Zero();
	
	sphere.AddRelTorque(impulses [1]);
	impulses[1] = Vector3::Zero();
}
