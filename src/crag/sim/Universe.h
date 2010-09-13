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


namespace sim
{
	// forward-declarations
	class Entity;
	
	
	// Currently deals with simulation time, entity managment and physics ticking.
	class Universe : public core::Singleton<Universe>
	{
	public:
		CONFIG_DECLARE_MEMBER (target_frame_seconds, sys::TimeType);
	
		Universe();
		~Universe();

		sys::TimeType GetTime() const;
		
		void ToggleGravity();

		void AddEntity(Entity & entity);

		void Tick();
		Vector3 Weight(Vector3 const & pos, Scalar mass) const;
		void GetRenderRange(Ray3 const & camera_ray, double & range_min, double & range_max, bool wireframe) const;
	};
}
