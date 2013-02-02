//
//  SpawnEntityFunctions.h
//  crag
//
//  Created by John McFarlane on 2013-02-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "sim/defs.h"

#include "gfx/Color.h"

namespace gfx
{
	DECLARE_CLASS_HANDLE(Object);// gfx::ObjectHandle
}

namespace sim
{
	DECLARE_CLASS_HANDLE(Entity);// sim::EntityHandle
}

sim::EntityHandle SpawnBall(const sim::Vector3 & position, gfx::Color4f color);
sim::EntityHandle SpawnBox(const sim::Vector3 & position, gfx::Color4f color);
sim::EntityHandle SpawnObserver(const sim::Vector3 & position);
sim::EntityHandle SpawnPlanet(const sim::Sphere3 & sphere, int random_seed, int num_craters);
gfx::ObjectHandle SpawnSkybox();
sim::EntityHandle SpawnStar();
sim::EntityHandle SpawnVehicle(sim::Vector3 const & position);

