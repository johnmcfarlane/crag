//
//  scripts/ga/AnimatController.cpp
//  crag
//
//  Created by John McFarlane on 2013-03-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "AnimatController.h"

#include "Thruster.h"

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "core/RosterObjectDefine.h"

using namespace sim;

CONFIG_DEFINE(animat_sensor_length, 5.f);

////////////////////////////////////////////////////////////////////////////////
// sim::AnimatController member definitions

CRAG_ROSTER_OBJECT_DEFINE(
	AnimatController,
	1,
	Pool::CallBetween<
		& AnimatController::Tick,
		Entity, & Entity::Tick,
		Sensor, & Sensor::GenerateScanRay>(Engine::GetTickRoster()))

AnimatController::AnimatController(Entity & entity, float radius)
: VehicleController(entity)
{
	CreateSensors(radius);
	CreateThrusters(radius);
	Connect();
}

void AnimatController::Tick()
{
}

void AnimatController::CreateSensors(float radius)
{
	auto const unit_component = static_cast<Scalar>(std::sqrt(1. / 3.));

	Ray3 ray;
	for (auto z = 0; z < 2; ++ z)
	{
		ray.direction.z = z ? unit_component : - unit_component;
		ray.position.z = ray.direction.z * radius;

		for (auto y = 0; y < 2; ++ y)
		{
			ray.direction.y = y ? unit_component : - unit_component;
			ray.position.y = ray.direction.y * radius;

			for (auto x = 0; x < 2; ++ x)
			{
				ray.direction.x = x ? unit_component : - unit_component;
				ray.position.x = ray.direction.x * radius;

				AddSensor(ray);
			}
		}
	}
}

void AnimatController::CreateThrusters(float radius)
{
	auto & entity = GetEntity();
	auto root_third = static_cast<float>(std::sqrt(1. / 3.) * radius);
	auto direction_scale = 5.f;

	Ray3 ray;
	for (auto z = 0; z < 2; ++ z)
	{
		ray.position.z = z ? root_third : -root_third;
		ray.direction.z = ray.position.z * direction_scale;

		for (auto y = 0; y < 2; ++ y)
		{
			ray.position.y = y ? root_third : -root_third;
			ray.direction.y = ray.position.y * direction_scale;

			for (auto x = 0; x < 2; ++ x)
			{
				ray.position.x = x ? root_third : -root_third;
				ray.direction.x = ray.position.x * direction_scale;

				AddReceiver(VehicleController::ReceiverPtr(new Thruster(entity, ray, false, 0.f)));
			}
		}
	}
}

void AnimatController::Connect()
{
}

void AnimatController::AddSensor(Ray3 const & ray)
{
	_sensors.emplace_back(new Sensor(GetEntity(), ray, animat_sensor_length, 0.2f));
}

void AnimatController::TickThrusters()
{
}
