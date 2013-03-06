//
//  scripts/ga/Sensor.h
//  crag
//
//  Created by John McFarlane on 2013-03-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "sim/defs.h"

namespace sim
{
	class Sensor
	{
	public:
		Sensor(Ray3 const & position);
	private:
		Ray3 _ray;	// Project(_ray, 1) = average sensor tip
		std::vector<float> _thruster_mapping;
	};
}
