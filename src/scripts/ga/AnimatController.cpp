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

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// sim::AnimatController member definitions

AnimatController::AnimatController(Entity & entity)
: VehicleController(entity)
{
	CreateSensors();
	CreateThrusters();
	Connect();
}

void AnimatController::CreateSensors()
{
	auto root_third = static_cast<float>(std::sqrt(1. / 3.));
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

				AddSensor(ray);
			}
		}
	}
}

void AnimatController::CreateThrusters()
{
	auto root_third = static_cast<float>(std::sqrt(1. / 3.));
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

				AddSensor(ray);
			}
		}
	}
}

void AnimatController::Connect()
{
}

void AnimatController::AddSensor(Ray3 const & ray)
{
	_sensors.emplace_back(ray);
}

void AnimatController::Tick()
{
}

void AnimatController::TickThrusters()
{
}
