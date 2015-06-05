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

#include "physics/RayCast.h"

#include "core/counted_object.h"

namespace crag
{
	   namespace core
	   {
			   class Roster;
	   }
}

namespace sim
{
	class Entity;

	class Sensor
	: public crag::counted_object<Sensor>
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		OBJECT_NO_COPY(Sensor);

		Sensor(Entity & entity, Ray3 const & position, Scalar length, Scalar variance = 0);

		CRAG_ROSTER_OBJECT_DECLARE(Sensor);

		Scalar GetReading() const;
		Scalar GetReadingDistance() const;

		CRAG_VERIFY_INVARIANTS_DECLARE(Sensor);

		void GenerateScanRay();
	private:
		Ray3 GetGlobalRay() const;

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
