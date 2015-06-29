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

#include "Health.h"
#include "Thruster.h"

#include <sim/Engine.h>

#include <core/RosterObjectDefine.h>

using namespace sim;

CONFIG_DEFINE(animat_sensor_length, 5.f);
CONFIG_DEFINE(animat_thruster_length, 25.f);
CONFIG_DEFINE(animat_thruster_gain_coefficient, -.0001f);

////////////////////////////////////////////////////////////////////////////////
// sim::AnimatController member definitions

CRAG_ROSTER_OBJECT_DEFINE(
	AnimatController,
	100,
	Pool::Call<& AnimatController::Tick>(Engine::GetTickRoster()))

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(AnimatController, self)
	CRAG_VERIFY(static_cast<VehicleController const &>(self));
CRAG_VERIFY_INVARIANTS_DEFINE_END

AnimatController::AnimatController(Entity & entity, float radius, ga::Genome && genome, HealthPtr && health)
: VehicleController(entity)
, _genome(std::move(genome))
, _health(std::move(health))
{
	CreateSensors(radius);
	CreateThrusters(radius);
	CreateNetwork();

	CRAG_VERIFY(* this);
}

ga::Genome const & AnimatController::GetGenome() const
{
	return _genome;
}

void AnimatController::Tick()
{
	auto sum_thrust = 0.f;

	for (auto & receiver : GetReceivers())
	{
		auto & thruster = core::StaticCast<Thruster const>(* receiver);
		sum_thrust += thruster.GetSignal();
	}

	_health->IncrementHealth(sum_thrust * animat_thruster_gain_coefficient);
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

	Ray3 ray;
	for (auto z = 0; z < 2; ++ z)
	{
		ray.position.z = z ? root_third : -root_third;
		ray.direction.z = ray.position.z * animat_thruster_length;

		for (auto y = 0; y < 2; ++ y)
		{
			ray.position.y = y ? root_third : -root_third;
			ray.direction.y = ray.position.y * animat_thruster_length;

			for (auto x = 0; x < 2; ++ x)
			{
				ray.position.x = x ? root_third : -root_third;
				ray.direction.x = ray.position.x * animat_thruster_length;

				CRAG_VERIFY_NEARLY_EQUAL(geom::Magnitude(ray), animat_thruster_length, .0001f);
				AddReceiver(VehicleController::ReceiverPtr(new Thruster(entity, ray, false, 0.f)));
			}
		}
	}
}

void AnimatController::CreateNetwork()
{
	auto transmitters = core::make_transform(GetTransmitters(), [] (TransmitterPtr const & sensor) {
		return static_cast<Transmitter *>(sensor.get());
	});
	transmitters.emplace(std::begin(transmitters), & _health->GetTransmitter());

	auto receivers = core::make_transform(GetReceivers(), [] (ReceiverPtr const & motor) {
		return static_cast<Receiver *>(motor.get());
	});

	_network = std::move(nnet::Network(_genome, std::vector<int>{
		int(transmitters.size()),
		10,
		int(receivers.size())}));
	_network.ConnectInputs(transmitters);
	_network.ConnectOutputs(receivers);
}

void AnimatController::AddSensor(Ray3 const & ray)
{
	AddTransmitter(TransmitterPtr(new Sensor(GetEntity(), ray, animat_sensor_length, 0.2f)));
}
