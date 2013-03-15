//
//  scripts/ga/Sensor.cpp
//  crag
//
//  Created by John McFarlane on 2013-03-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Sensor.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// sim::Sensor member definitions

DISABLE_ALLOCATOR(Sensor);

Sensor::Sensor(Ray3 const & ray)
: _ray(ray)
{
}
