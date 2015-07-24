//
//  scripts/ga/AnimatController.h
//  crag
//
//  Created by John McFarlane on 2013-03-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "entity/sim/VehicleController.h"

#include "Genome.h"
#include "nnet.h"
#include "Sensor.h"

namespace sim
{
	class Health;

	// governs the behevior of a sim::Entity which is an artificial animal
	class AnimatController : public VehicleController
	{
	public:
		// types
		using HealthPtr = std::unique_ptr<Health>;

		////////////////////////////////////////////////////////////////////////////////
		// functions

		CRAG_ROSTER_OBJECT_DECLARE(AnimatController);
		CRAG_VERIFY_INVARIANTS_DECLARE(AnimatController);

		AnimatController(Entity & entity, ga::Genome && genome, HealthPtr && health);

		ga::Genome const & GetGenome() const;
	private:
		void Tick();

		void CreateSensors(ga::GenomeReader & genome_reader);
		void CreateThrusters(ga::GenomeReader & genome_reader, Entity & entity);
		void CreateNetwork(ga::GenomeReader & genome_reader);

		void AddSensor(Ray3 const & ray, Scalar length);

		////////////////////////////////////////////////////////////////////////////////
		// variables

		ga::Genome _genome;
		nnet::Network _network;
		HealthPtr _health;
	};
}
