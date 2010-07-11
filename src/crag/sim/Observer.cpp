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
#include "Space.h"

#include "physics/Singleton.h"
#include "physics/SphericalBody.h"

#include "geom/VectorOps.h"
#include "geom/MatrixOps.h"
#include "geom/Vector4.h"
#include "core/ConfigEntry.h"

#include "gfx/Debug.h"


namespace 
{
	CONFIG_DEFINE (camera_radius, float, 1);
	CONFIG_DEFINE (camera_density, float, 1);
	
	CONFIG_DEFINE (camera_speed_factor, float, 631);

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
, sphere(true, camera_radius)
, speed(0)
, speed_factor(camera_speed_factor)
, light(Vector3::Zero(), observer_light_color, observer_light_attenuation_a, observer_light_attenuation_b, observer_light_attenuation_c)
{
	sphere.SetDensity(camera_density);
	sphere.SetLinearDamping(camera_linear_damping);
	sphere.SetAngularDamping(camera_angular_damping);

	impulses[0] = impulses[1] = Vector3::Zero();
}

sim::Observer::~Observer()
{
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
	
	Scalar inv_t = 1.f / Universe::target_frame_seconds;

	for (int d = 0; d < 2; ++ d)
	{
		for (int axis = 0; axis < 3; ++ axis)
		{
			Scalar factor = impulse.factors[d][axis];

			impulses[d][axis] = impulse_factors[d][axis] * factor * inv_t;
		}
	}
	
	// Debug out
	if (gfx::Debug::GetVerbosity() > .8) {
		for (int n = 0; n < 2; ++ n) {
			gfx::Debug::out << (n ? "torque:" : "force: ");
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
				gfx::Debug::out << c;
			}
			gfx::Debug::out << '\n';
		}
	}
}

/*void sim::Observer::SetSpeed(int _speed)
{
	speed = _speed;
}*/

void sim::Observer::SetSpeedFactor(int _speed_factor)
{
	// TODO: Specify a range in config.
	speed_factor = static_cast<float>(Power(Power(10., .4), static_cast<double>((_speed_factor << 1) + 1)));
}

void sim::Observer::Tick()
{
	ApplyImpulse();

	float mass = 1;
	Vector3 const & position = GetPosition();
	Vector3 gravitational_force = Universe::Weight(position, mass);
	sphere.AddRelForceAtRelPos(gravitational_force / static_cast<Scalar>(Universe::target_frame_seconds), Vector3(0, .1, -0.45));
	
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
	return sphere.GetPosition();
}

void sim::Observer::SetPosition(sim::Vector3 const & pos) 
{
	light.SetPosition(pos);
	sphere.SetPosition(pos);
}

physics::Body * sim::Observer::GetBody()
{
	return & sphere;
}

physics::Body const * sim::Observer::GetBody() const
{
	return & sphere;
}

gfx::Light const & sim::Observer::GetLight() const
{
	return light;
}

void sim::Observer::ApplyImpulse()
{
	sphere.AddRelForce(impulses [0]);
	sphere.AddRelTorque(impulses [1]);
}
