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

#include "core/App.h"

#include "core/ConfigEntry.h"


namespace sim
{
	class Entity;
	
	
	// TODO: Should probably be a singleton and also is a bit of a waste of space.
	namespace Universe
	{
		extern app::TimeType time;

		CONFIG_DECLARE (target_frame_seconds, app::TimeType);
	
		void Init();
		void Deinit();

		void ToggleGravity();

		void AddEntity(Entity & entity);

		void Tick();
		sim::Vector3 Weight(Vector3 const & pos, float mass);
		void GetRenderRange(Ray3 const & camera_ray, double & range_min, double & range_max);
	}
}
