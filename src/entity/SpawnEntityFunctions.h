//
//  SpawnEntityFunctions.h
//  crag
//
//  Created by John McFarlane on 2013-02-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "sim/defs.h"

#include "geom/Space.h"

namespace gfx
{
	template <typename> class Color4;
	typedef Color4<float> Color4f;
}

sim::EntityHandle SpawnBall(sim::Sphere3 const & sphere, sim::Vector3 const & velocity, gfx::Color4f color);
sim::EntityHandle SpawnBox(sim::Vector3 const & position, sim::Vector3 const & velocity, sim::Vector3 const & size, gfx::Color4f color);
sim::EntityHandle SpawnCamera(sim::Vector3 const & position, sim::EntityHandle subject);
sim::EntityHandle SpawnPlanet(sim::Sphere3 const & sphere, int random_seed, int num_craters);
sim::EntityHandle SpawnStar(geom::abs::Sphere3 const & volume, gfx::Color4f const & color, bool casts_shadow = true);
