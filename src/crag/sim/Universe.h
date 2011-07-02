/*
 *  Universe.h
 *  Crag
 *
 *  Created by john on 4/14/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "sim/defs.h"

#include "sys/App.h"

#include "core/ConfigEntry.h"
#include "core/Singleton.h"


namespace physics 
{
	class Body;
}


namespace sim
{
	// forward-declarations
	class Entity;
	
	
	// Currently deals with simulation time, entity managment and physics ticking.
	class Universe
	{
	public:
		CONFIG_DECLARE_MEMBER (gravitational_force, float);
		CONFIG_DECLARE_MEMBER (gravity, bool);

		// methods
		Universe();
		~Universe();

		sys::TimeType GetTime() const;
		
		void ToggleGravity();

		void AddEntity(Entity & entity);
		void RemoveEntity(Entity & entity);

		void Tick(sys::TimeType target_frame_seconds);
		Vector3 Weight(Vector3 const & pos, Scalar mass) const;
		void ApplyGravity(physics::Body & body) const;
		void ApplyGravity(physics::Body & body, Vector3 const & center_of_mass) const;

	private:
		// attributes
		sys::TimeType time;
		
		typedef std::vector<Entity *> EntityVector;
		EntityVector entities;
	};
}
