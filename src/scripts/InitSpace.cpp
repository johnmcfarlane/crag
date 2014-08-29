//
//  InitSpace.cpp
//  crag
//
//  Created by John McFarlane on 2014-07-18.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "InitSpace.h"
#include "MonitorOrigin.h"
#include "RegulatorScript.h"

#include "sim/Engine.h"

#include "form/Engine.h"
#include "form/Scene.h"

#include "gfx/axes.h"
#include "gfx/Engine.h"
#include "gfx/SetCameraEvent.h"
#include "gfx/SetLodParametersEvent.h"
#include "gfx/SetSpaceEvent.h"

#include "applet/Applet.h"
#include "applet/AppletInterface_Impl.h"

#include "core/ConfigEntry.h"

CONFIG_DECLARE(origin_dynamic_enable, bool);
CONFIG_DECLARE_ANGLE(frustum_default_fov, float);

// main entry point
void InitSpace(applet::AppletInterface & applet_interface, geom::Space const & space)
{
	FUNCTION_NO_REENTRY;

	// Create origin controller.
	if (origin_dynamic_enable)
	{
		applet_interface.Launch("MonitorOrigin", 8192, &MonitorOrigin);
	}
	
	// launch regulator
	applet_interface.Launch("Regulator", 8192, & RegulatorScript);
	
	// Set Space
	{
		gfx::SetSpaceEvent event = 
		{
			space
		};
		applet::Daemon::Broadcast(event);
	}
	
	// set LOD parameters
	{
		gfx::SetLodParametersEvent event;
		event.parameters.center = gfx::Vector3::Zero();
		event.parameters.min_distance = 1.f;
		gfx::Daemon::Broadcast(event);
	}
	
	// Set camera position
	{
		gfx::SetCameraEvent event = { 
			{ geom::abs::Vector3::Zero() },
			frustum_default_fov
		};
		gfx::Daemon::Broadcast(event);
	}

	// Give formations time to expand.
	while (applet_interface.Get<sim::Engine, bool>([] (sim::Engine const & engine) {
		return ! engine.IsSettled();
	}));
	
	while (applet_interface.Get<form::Engine, bool>([] (form::Engine const & engine) {
		return ! engine.IsSettled();
	}));
}
