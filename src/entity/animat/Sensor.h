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

namespace core
{
	   namespace locality
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
		DECLARE_ALLOCATOR(Sensor);

		Sensor(Entity & entity, Ray3 const & position, Scalar length, Scalar variance = 0);
		~Sensor();
		
		Scalar GetReading() const;
		Scalar GetReadingDistance() const;

		CRAG_VERIFY_INVARIANTS_DECLARE(Sensor);

	private:
		void Tick();
		Ray3 GetGlobalRay() const;
		void GenerateScanRay() const;

		core::locality::Roster & GetTickRoster();

		////////////////////////////////////////////////////////////////////////////////
		// variables

		Entity & _entity;
		Scalar _length;
		Scalar _variance;
		physics::RayCast & _ray_cast;
		Ray3 const _local_ray;	// Project(_ray, 1) = average sensor tip
		std::vector<float> _thruster_mapping;
	};
}
