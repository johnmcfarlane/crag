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

#include "gfx/axes.h"
#include "gfx/Engine.h"
#include "gfx/SetCameraEvent.h"
#include "gfx/SetLodParametersEvent.h"
#include "gfx/SetOriginEvent.h"

#include "applet/Applet.h"
#include "applet/AppletInterface_Impl.h"

#include "core/ConfigEntry.h"

CONFIG_DECLARE(origin_dynamic_enable, bool);

// main entry point
void InitSpace(applet::AppletInterface & applet_interface, geom::abs::Vector3 const & start_pos)
{
	FUNCTION_NO_REENTRY;

	// coordinate system
	auto rel_start_pos = geom::AbsToRel(start_pos, start_pos);
	
	// Create origin controller.
	if (origin_dynamic_enable)
	{
		applet_interface.Launch("MonitorOrigin", 8192, &MonitorOrigin);
	}
	
	// launch regulator
	applet_interface.Launch("Regulator", 8192, & RegulatorScript);
	
	// Set Origin
	{
		gfx::SetOriginEvent event = 
		{
			start_pos
		};
		applet::Daemon::Broadcast(event);
	}
	
	// set LOD parameters
	{
		gfx::SetLodParametersEvent event;
		event.parameters.center = rel_start_pos;
		event.parameters.min_distance = 1.f;
		gfx::Daemon::Broadcast(event);
	}
	
	// Set camera position
	{
		gfx::SetCameraEvent event;
		event.transformation.SetTranslation(start_pos * 3.);
		event.transformation.SetRotation(gfx::Rotation(geom::abs::Vector3(0, 0, 1)));
		gfx::Daemon::Broadcast(event);
	}
}
