//
//  SpawnEntityFunctions.h
//  crag
//
//  Created by John McFarlane on 2013-02-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "sim/defs.h"

namespace gfx
{
	DECLARE_CLASS_HANDLE(Object);// gfx::ObjectHandle
	
	template <typename> class Color4;
	typedef Color4<float> Color4f;
}

sim::EntityHandle SpawnBall(const sim::Vector3 & position, sim::Scalar radius, gfx::Color4f color);
sim::EntityHandle SpawnBox(const sim::Vector3 & position, sim::Vector3 const & size, gfx::Color4f color);
sim::EntityHandle SpawnObserver(const sim::Vector3 & position);
sim::EntityHandle SpawnPlanet(const sim::Sphere3 & sphere, int random_seed, int num_craters);
gfx::ObjectHandle SpawnSkybox();
sim::EntityHandle SpawnStar();
sim::EntityHandle SpawnRover(sim::Vector3 const & position);

