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

#include "sim/Engine.h"

#include "physics/Engine.h"

#include "form/Engine.h"
#include "form/node/NodeBuffer.h"

#include "geom/origin.h"

#include "applet/AppletInterface_Impl.h"

#include "core/app.h"
#include "core/ConfigEntry.h"

CONFIG_DEFINE (min_precision_score, sim::Scalar, .001f);

namespace
{
	// Given the camera position relative to the current origin
	// and the distance to the closest bit of geometry,
	// is the origin too far away to allow for precise, camera-centric calculations?
	bool ShouldReviseOrigin(geom::rel::Vector3 const & camera_pos, float min_leaf_distance_squared)
	{
		if (min_leaf_distance_squared == std::numeric_limits<decltype(min_leaf_distance_squared)>::max())
		{
			return true;
		}
		
		ASSERT(min_leaf_distance_squared >= 0);
		auto distance_from_surface = std::sqrt(min_leaf_distance_squared);

		auto distance_from_origin = geom::Length(camera_pos);
		
		auto precision_score = distance_from_surface / distance_from_origin;
		if (precision_score < min_precision_score)
		{
			return true;
		}

		return false;
	}
	
	void ReviseOrigin(sim::Engine & engine)
	{
		auto& physics_engine = engine.GetPhysicsEngine();
		auto& scene = physics_engine.GetScene();
		auto& node_buffer = scene.GetNodeBuffer();

		auto& camera_ray = engine.GetCamera();
		auto& camera_pos = camera_ray.position;
		auto min_leaf_distance_squared = node_buffer.GetMinLeafDistanceSquared();

		if (ShouldReviseOrigin(camera_pos, min_leaf_distance_squared))
		{
	#if ! defined(NDEBUG)
			app::Beep();
	#endif

			auto origin = engine.GetOrigin();
			auto new_origin = geom::RelToAbs(camera_pos, origin);

			DEBUG_MESSAGE("Set: %f,%f,%f", new_origin.x, new_origin.y, new_origin.z);
			engine.SetOrigin(new_origin);
		}
	}
}

void MonitorOrigin(applet::AppletInterface & applet_interface)
{
	while (! applet_interface.GetQuitFlag())
	{
		applet_interface.Sleep(0.23432);
		sim::Daemon::Call(& ReviseOrigin);
	}
}
