//
//  SpawnPlayer.h
//  crag
//
//  Created by John McFarlane on 2014-04-13.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "sim/defs.h"

namespace geom
{
	class Space;
}

sim::EntityHandle SpawnRover(sim::Vector3 const & position, sim::Scalar thrust);
std::array<sim::EntityHandle, 2> SpawnPlayer(sim::Vector3 const & translation, geom::Space const & space);
