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

#include <gfx/axes.h>

#include <geom/Intersection.h>

#include <core/Random.h>
#include <core/RosterObjectDefine.h>

using namespace sim;

CONFIG_DEFINE(animat_sensor_length, 5.f);
CONFIG_DEFINE(animat_sensor_count, 4);
CONFIG_DEFINE(animat_sensor_embed_coefficient, .8f);

CONFIG_DEFINE(animat_thruster_length, 15.f);
CONFIG_DEFINE(animat_thruster_gain_coefficient, -.0002f);
CONFIG_DEFINE(animat_thruster_count, 4);

namespace
{
	Ray3 ShiftedToSphereSurface(Ray3 const & ray)
	{
		Scalar t1, t2;
		if (! geom::GetIntersection(Sphere3(Vector3::Zero(), 1), ray, t1, t2))
		{
			CRAG_DEBUG_DUMP(ray);
			DEBUG_BREAK("GetIntersection failed:");
			return ray;
		}

		CRAG_VERIFY_OP(t1, <=, .001f);
		CRAG_VERIFY_OP(t2, >=, 0.f);
		return Ray3(
			geom::Project(ray, t1) * animat_sensor_embed_coefficient,
			ray.direction);
	}

	Ray3 ReadRay(ga::GenomeReader & genome_reader)
	{
		auto read_signed_unit = [&] ()
		{
			return genome_reader.Read().closed() * 2.f - 1.f;
		};
		auto read_vector = [&]()
		{
			return Vector3(read_signed_unit(), read_signed_unit(), read_signed_unit());
		};

		auto from = read_vector();
		auto to = read_vector();

		for (; ;)
		{
			auto rejiggered = [](Vector3 const & v)
			{
				return geom::Clamped(v + gfx::RandomVector<Scalar>(Random::sequence) * .01f, 1);
			};

			Ray3 ray;
			ray.position = rejiggered(from);
			ray.direction = rejiggered(to) - ray.position;
			if (ray.direction != Vector3::Zero())
			{
				return geom::Normalized(ray);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// sim::AnimatController member definitions

CRAG_ROSTER_OBJECT_DEFINE(
	AnimatController,
	100,
	Pool::Call<& AnimatController::Tick>(Engine::GetTickRoster()))

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(AnimatController, self)
	CRAG_VERIFY(static_cast<VehicleController const &>(self));
CRAG_VERIFY_INVARIANTS_DEFINE_END

AnimatController::AnimatController(Entity & entity, ga::Genome && genome, HealthPtr && health)
: VehicleController(entity)
, _genome(std::move(genome))
, _health(std::move(health))
{
	ga::GenomeReader genome_reader(_genome);

	CreateSensors(genome_reader);
	CreateThrusters(genome_reader, entity);
	CreateNetwork(genome_reader);

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

void AnimatController::CreateSensors(ga::GenomeReader & genome_reader)
{
	for (auto i = animat_sensor_count; i; -- i)
	{
		AddSensor(
			ShiftedToSphereSurface(ReadRay(genome_reader)),
			genome_reader.Read().closed() * animat_sensor_length);
	}
}

void AnimatController::CreateThrusters(ga::GenomeReader & genome_reader, Entity & entity)
{
	for (auto i = animat_sensor_count; i; -- i)
	{
		AddReceiver(VehicleController::ReceiverPtr(new Thruster(
			entity,
			ReadRay(genome_reader) * animat_thruster_length,
			false,
			0.f)));
	}
}

void AnimatController::CreateNetwork(ga::GenomeReader & genome_reader)
{
	auto transmitters = core::make_transform(GetTransmitters(), [] (TransmitterPtr const & sensor) {
		return static_cast<Transmitter *>(sensor.get());
	});
	transmitters.emplace(std::begin(transmitters), & _health->GetTransmitter());

	auto receivers = core::make_transform(GetReceivers(), [] (ReceiverPtr const & motor) {
		return static_cast<Receiver *>(motor.get());
	});

	_network = std::move(nnet::Network(genome_reader, std::vector<int>{
		int(transmitters.size()),
		10,
		int(receivers.size())}));
	_network.ConnectInputs(transmitters);
	_network.ConnectOutputs(receivers);
}

void AnimatController::AddSensor(Ray3 const & ray, Scalar length)
{
	AddTransmitter(TransmitterPtr(new Sensor(GetEntity(), ray, length, 0.2f)));
}
