//
//  scripts/ga/Sensor.h
//  crag
//
//  Created by John McFarlane on 2013-03-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "sim/defs.h"

namespace crag
{
	   namespace core
	   {
			   class Roster;
	   }
}

namespace physics
{
	class RayCast;
}

namespace sim
{
	class Entity;

	class Sensor
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		OBJECT_NO_COPY(Sensor);

		Sensor(Entity & entity, Ray3 const & position, Scalar length, Scalar variance = 0);
		~Sensor();
		
		Scalar GetReading() const;
		Scalar GetReadingDistance() const;

		CRAG_VERIFY_INVARIANTS_DECLARE(Sensor);

		static void Tick(Sensor *);
	private:
		Ray3 GetGlobalRay() const;
		void GenerateScanRay() const;

		crag::core::Roster & GetTickRoster();

		////////////////////////////////////////////////////////////////////////////////
		// variables

		Entity & _entity;
		Scalar _length;
		Scalar _variance;
		std::unique_ptr<physics::RayCast> const _ray_cast;
		Ray3 const _local_ray;	// Project(_ray, 1) = average sensor tip
		std::vector<float> _thruster_mapping;
	};
}
