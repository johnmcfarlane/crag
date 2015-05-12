//
//  MonitorOrigin.cpp
//  crag
//
//  Created by John McFarlane on 2013-01-22.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "MonitorOrigin.h"

#include "form/Engine.h"

#include "gfx/SetSpaceEvent.h"

#include "geom/Space.h"

#include "applet/AppletInterface.h"
#include "applet/Engine.h"

#include "core/ConfigEntry.h"
#include "core/Statistics.h"

CONFIG_DECLARE (observer_use_touch, bool);
CONFIG_DEFINE(origin_dynamic_enable, true);

namespace
{
	CONFIG_DEFINE(min_precision_score, .001f);
	CONFIG_DEFINE(origin_touch_max_distance, 10000.f);
   
	STAT_DEFAULT(sim_origin_reset_distance_factor, form::Scalar, 0.6f, 0.f);
	STAT_DEFAULT(sim_origin_reset_precision_factor, form::Scalar, 0.3f, 0.f);

	// Given the camera position relative to the current origin
	// and the distance to the closest bit of geometry,
	// is the origin too far away to allow for precise, camera-centric calculations?
	bool ShouldReviseOrigin(geom::rel::Vector3 const & lod_center, float min_leaf_distance_squared)
	{
		auto distance_from_origin = geom::Magnitude(lod_center);
		
		STAT_SET(sim_origin_reset_distance_factor, -1.f);
		STAT_SET(sim_origin_reset_precision_factor, -1.f);
	   
		// especially on Android, certain things go wrong when the origin is too far away
		if (observer_use_touch)
		{
			STAT_SET(sim_origin_reset_distance_factor, distance_from_origin / origin_touch_max_distance);

			if (distance_from_origin > origin_touch_max_distance)
			{
				return true;
			}
		}
		
		if (min_leaf_distance_squared == std::numeric_limits<decltype(min_leaf_distance_squared)>::max())
		{
			return false;
		}
		
		ASSERT(min_leaf_distance_squared >= 0);
		auto distance_from_surface = std::sqrt(min_leaf_distance_squared);

		auto precision_score = distance_from_surface / distance_from_origin;
		STAT_SET(sim_origin_reset_precision_factor, min_precision_score / precision_score);

		if (precision_score < min_precision_score)
		{
			return true;
		}

		return false;
	}
	
	void ReviseOrigin(form::Engine & engine)
	{
		auto empty_space = engine.GetEmptySpace();

		if (ShouldReviseOrigin(empty_space.center, empty_space.radius))
		{
#if defined(CRAG_DEBUG)
			app::Beep();
#endif

			auto space = engine.GetSpace();

			auto origin = space.RelToAbs(empty_space.center);
			DEBUG_MESSAGE("Set: %lf,%lf,%lf", origin.x, origin.y, origin.z);

			gfx::SetSpaceEvent event = { geom::Space(origin) };
			applet::Daemon::Broadcast(event);
		}
	}
}

void MonitorOrigin(applet::AppletInterface & applet_interface)
{
	ASSERT(origin_dynamic_enable);
	
	while (! applet_interface.GetQuitFlag())
	{
		applet_interface.WaitFor(0.23432);
		form::Daemon::Call(& ReviseOrigin);
	}
}
