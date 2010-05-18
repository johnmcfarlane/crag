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

#include "Observer.h"
#include "Universe.h"
#include "Physics.h"
#include "Space.h"

#include "core/VectorOps.h"
#include "core/MatrixOps.h"
#include "core/Vector4.h"
#include "core/ConfigEntry.h"

#include "gfx/DebugGraphics.h"


namespace ANONYMOUS {

//sim::Vector3 const default_camera_pos(0, - 1500000, 0);
CONFIG_DEFINE (camera_speed_factor, float, 1);

CONFIG_DEFINE (camera_linear_damping, float, 0.04f);
CONFIG_DEFINE (camera_angular_damping, float, 0.1f);
CONFIG_DEFINE (camera_velocity_impulse, float, 0.001f);
CONFIG_DEFINE (camera_torque_impulse, float, .1f);

CONFIG_DEFINE (observer_light_color, gfx::Color4f, gfx::Color4f(0.8f, 0.8f, 1.0f));
CONFIG_DEFINE (observer_light_attenuation_a, float, 0.00000001f);
CONFIG_DEFINE (observer_light_attenuation_b, float, 0.000f);
CONFIG_DEFINE (observer_light_attenuation_c, float, 4.000f);

}


sim::Observer::Observer()
: Entity()
, sphere()
, speed(0)
, speed_factor(camera_speed_factor)
, light(Vector3::Zero(), observer_light_color, observer_light_attenuation_a, observer_light_attenuation_b, observer_light_attenuation_c)
{
	sphere.Init(* this, 1, 1);

	dBodyID body = sphere.GetBody();
	dBodySetLinearDamping(body, camera_linear_damping);
	dBodySetAngularDamping(body, camera_angular_damping);

	impulses[0] = impulses[1] = Vector3::Zero();
}

sim::Observer::~Observer()
{
//	camera_pos = GetPosition();
//	camera_rot = GetRotation();
	camera_speed_factor = static_cast<float>(speed_factor);
}

void sim::Observer::UpdateInput(Controller::Impulse const & impulse)
{
	Scalar velocity_impulse = camera_velocity_impulse * speed_factor;
	Vector3 const impulse_factors[2] = 
	{
		Vector3(velocity_impulse, velocity_impulse, velocity_impulse),
		Vector3(camera_torque_impulse, camera_torque_impulse, camera_torque_impulse),
	};
	
	Scalar inv_t = 1.f / Universe::target_frame_period;

	for (int d = 0; d < 2; ++ d)
	{
		for (int axis = 0; axis < 3; ++ axis)
		{
			Scalar factor = impulse.factors[d][axis];

			impulses[d][axis] = impulse_factors[d][axis] * factor * inv_t;
		}
	}
	
	// Debug out
	if (gfx::DebugGraphics::GetVerbosity() > .8) {
		for (int n = 0; n < 2; ++ n) {
			gfx::DebugGraphics::out << (n ? "torque:" : "force: ");
			for (int m = 0; m < 3; ++ m) {
				char c;
				int f = static_cast<int>(impulse.factors[n][m]);
				if (f < 0) {
					c = '-';
				}
				else if (f == 0) {
					c = '0';
				}
				else if (f > 0) {
					c = '+';
				}
				gfx::DebugGraphics::out << c;
			}
			gfx::DebugGraphics::out << '\n';
		}
	}
}

/*void sim::Observer::SetSpeed(int _speed)
{
	speed = _speed;
}*/

void sim::Observer::SetSpeedFactor(int _speed_factor)
{
	speed_factor = static_cast<float>(Power(Power(10., .3), static_cast<double>((_speed_factor << 1) + 1)));
}

void sim::Observer::Tick()
{
	ApplyImpulse();

	float mass = 1;
	Vector3 const & position = GetPosition();
	Vector3 gravitational_force = Universe::Weight(position, mass);
	dBodyID body = sphere.GetBody();
	AddRelForceAtRelPos(body, gravitational_force / static_cast<Scalar>(Universe::target_frame_period), Vector3(0, .1, -0.45));
	
	light.SetPosition(GetPosition());
}

sim::Vector3 const * sim::Observer::GetImpulse() const 
{ 
	return impulses; 
}

sim::Scalar sim::Observer::GetBoundingRadius() const
{
	return sphere.GetRadius();
}

sim::Vector3 const & sim::Observer::GetPosition() const
{
	dBodyID const body = sphere.GetBody();
	return sim::GetPosition(body);
}

void sim::Observer::SetPosition(sim::Vector3 const & pos) 
{
	light.SetPosition(pos);
	
	dBodyID const body = sphere.GetBody();
	sim::SetPosition(body, pos);
}

sim::Matrix4 sim::Observer::GetRotation() const
{
	Matrix4 m;

	dBodyID body = sphere.GetBody();
	sim::GetRotation(body, m);
	
	return m;
}

void sim::Observer::SetRotation(sim::Matrix4 const & rot)
{
	dBodyID body = sphere.GetBody();
	sim::SetRotation(body, rot);
}

sim::PhysicalBody * sim::Observer::GetPhysicalBody()
{
	return & sphere;
}

gfx::Light const & sim::Observer::GetLight() const
{
	return light;
}

void sim::Observer::ApplyImpulse()
{
	dBodyID body = sphere.GetBody();
	AddRelForce(body, impulses [0]);
	AddRelTorque(body, impulses [1]);
}
