/*
 *  Universe.cpp
 *  Crag
 *
 *  Created by john on 4/28/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Universe.h"
//#include "form/Manager.h"
#include "gfx/IndexBuffer.h"
#include "form/VertexBuffer.h"
#include "Entity.h"
#include "core/VectorOps.h"
#include "Firmament.h"

#include "physics/Singleton.h"

#include <algorithm>
#include <vector>


//////////////////////////////////////////////////////////////////////
// local definitions

namespace ANONYMOUS {

CONFIG_DEFINE (gravity, bool, true);

typedef std::vector<sim::Entity *> EntityList;
EntityList entities;

}


//////////////////////////////////////////////////////////////////////
// Universe definitions

app::TimeType sim::Universe::time = 0;

CONFIG_DEFINE_MEMBER (sim::Universe, target_frame_seconds, double, 1.f / 60.f);

CONFIG_DEFINE (gravitational_force, float, 0.000000001f);


void sim::Universe::Init()
{
	time = 0;
	
	Assert(entities.size() == 0);

	//form::Manager::Init();
}

void sim::Universe::Deinit()
{
	for (EntityList::iterator it = entities.begin(); it != entities.end(); ++ it)
	{
		Entity * e = * it;
		delete e;
	}
	entities.clear();

	//form::Manager::Deinit();
}

void sim::Universe::ToggleGravity()
{
	gravity = ! gravity;
}

void sim::Universe::AddEntity(Entity & entity)
{
	Assert(std::find(entities.begin(), entities.end(), & entity) == entities.end());
	entities.push_back(& entity);
}

void sim::Universe::Tick()
{
	time += target_frame_seconds;
	physics::Singleton::Get().Tick(target_frame_seconds);
	
	for (EntityList::const_iterator it = entities.begin(); it != entities.end(); ++ it)
	{
		Entity & e = * * it;
		e.Tick();
	}
}

sim::Vector3 sim::Universe::Weight(sim::Vector3 const & pos, float mass)
{
	if (gravity) {
		sim::Vector3 force = sim::Vector3::Zero();
		
		for (EntityList::const_iterator it = entities.begin(); it != entities.end(); ++ it) {
			Entity & e = * * it;
			e.GetGravitationalForce(pos, force);
		}
		
		return force * static_cast<sim::Scalar>(mass) * static_cast<sim::Scalar>(gravitational_force);
	}
	else {
		return sim::Vector3::Zero();
	}
}
