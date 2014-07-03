//
//  SpawnPlayer.h
//  crag
//
//  Created by John McFarlane on 2014-04-13.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "sim/defs.h"

enum class PlayerType
{
	observer,
	arrow,
	thargoid,
	cos_saucer,
	ball_saucer
};

sim::EntityHandle SpawnRover(sim::Vector3 const & position, sim::Scalar thrust);
sim::EntityHandle SpawnPlayer(sim::Vector3 const & position, PlayerType player_type);
