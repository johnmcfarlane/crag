//
//  scripts/ga/Sensor.h
//  crag
//
//  Created by John McFarlane on 2013-03-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Signal.h"

#include "sim/defs.h"

#include "physics/RayCast.h"

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

	class Sensor final
		: public Transmitter
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		OBJECT_NO_COPY(Sensor);

		Sensor(Entity & entity, Ray3 const & position, Scalar length, Scalar variance = 0);

		CRAG_ROSTER_OBJECT_DECLARE(Sensor);
		CRAG_VERIFY_INVARIANTS_DECLARE(Sensor);

		void GenerateScanRay() noexcept;
	private:
		void SendReading() noexcept;

#if defined(CRAG_DEBUG)
		void DebugDraw() noexcept;
#endif

		Scalar CalcReading() const;
		Ray3 GetGlobalRay() const;

		////////////////////////////////////////////////////////////////////////////////
		// variables

		Entity & _entity;
		Scalar _length;
		Scalar _variance;
		std::unique_ptr<physics::RayCast> const _ray_cast;
		Ray3 const _local_ray;    // Project(_ray, 1) = average sensor tip
		std::vector<float> _thruster_mapping;
	};
}
