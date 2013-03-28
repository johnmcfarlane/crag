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

namespace sim
{
	class Controller;

	class Sensor
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		DECLARE_ALLOCATOR(Sensor);

		Sensor(Controller & controller, Ray3 const & position);
		~Sensor();

	private:
		void Tick();

		core::locality::Roster & GetTickRoster();

		////////////////////////////////////////////////////////////////////////////////
		// variables

		Controller & _controller;
		Ray3 _ray;	// Project(_ray, 1) = average sensor tip
		std::vector<float> _thruster_mapping;
	};
}
