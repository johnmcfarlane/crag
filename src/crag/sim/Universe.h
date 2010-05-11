/*
 *  Universe.h
 *  Crag
 *
 *  Created by john on 4/14/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "sim/Defs.h"
#include "core/ConfigEntry.h"


namespace sim
{
	class Entity;
	
	
	// TODO: Should probably be a singleton and also is a bit of a waste of space.
	namespace Universe
	{
		extern float time;
		//extern float delta_time;

		CONFIG_DECLARE (target_frame_period, float);
		CONFIG_DECLARE (frame_time_error, float);
	
		void Init();
		void Deinit();

		void ToggleGravity();

		void AddEntity(Entity & entity);

		void Tick();
		sim::Vector3 Weight(sim::Vector3 const & pos, float mass);
	}
}
