//
//  MainScript.cpp
//  crag
//
//  Created by John McFarlane on 2013-02-15.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "SpawnEntityFunctions.h"
#include "TestScript.h"
#include "MonitorOrigin.h"
#include "RegulatorScript.h"

#include "applet/Applet.h"
#include "applet/AppletInterface_Impl.h"
#include "applet/Engine.h"

#include "sim/axes.h"
#include "sim/Engine.h"
#include "sim/Entity.h"
#include "sim/Firmament.h"

#include "physics/Location.h"

#include "form/Engine.h"
#include "form/node/NodeBuffer.h"

#include "gfx/Engine.h"

#include "core/app.h"

#include "core/app.h"
#include "core/EventWatcher.h"
#include "core/Random.h"

using geom::Vector3f;

namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// types
	typedef std::vector<sim::EntityHandle> EntityVector;
		
	////////////////////////////////////////////////////////////////////////////////
	// setup variables
	
	geom::rel::Vector3 observer_start_pos(0, 9999400, -5);
	
	////////////////////////////////////////////////////////////////////////////////
	// variables

	applet::AppletInterface * _applet_interface;
	core::EventWatcher _event_watcher;
	bool _enable_dynamic_origin = true;
	
	////////////////////////////////////////////////////////////////////////////////
	// functions
	
	sim::EntityHandle SpawnAnimat(/*const sim::Vector3 & position*/)
	{
		return sim::EntityHandle();
	}

	void SpawnAnimats(applet::AppletInterface & applet_interface)
	{
		auto num_animats = 10;
		sim::EntityHandle animats[num_animats];
		for (auto& animat : animats)
		{
			animat = SpawnAnimat();
		}
	}

	// returns true if the applet should NOT quit
	bool HandleEvents(sim::EntityHandle observer)
	{
		int num_events = 0;
	
		SDL_Event event;
		while (_event_watcher.PopEvent(event))
		{
			++ num_events;
		
			if (event.type != SDL_KEYDOWN)
			{
				continue;
			}

			switch (event.key.keysym.scancode)
			{
				case SDL_SCANCODE_ESCAPE:
				{
					return false;
				}

				case SDL_SCANCODE_I:
					if ((event.key.keysym.mod & KMOD_CTRL) == 0)
					{
						break;
					}
				
					if ((event.key.keysym.mod & (KMOD_SHIFT | KMOD_ALT | KMOD_CTRL)) != 0)
					{
						break;
					}
				
					_enable_dynamic_origin = ! _enable_dynamic_origin;
					break;
				
				default:
					break;
			}
		}

		return true;
	}
}

// main entry point
void MainScript(applet::AppletInterface & applet_interface)
{
	FUNCTION_NO_REENTRY;

	_applet_interface = & applet_interface;
	
	// Set camera position
	{
		sim::Transformation transformation(geom::Cast<sim::Scalar>(observer_start_pos));
		gfx::Daemon::Call([transformation] (gfx::Engine & engine) {
			engine.OnSetCamera(transformation);
		});
	}
	
	// Create sun. 
	sim::EntityHandle sun = SpawnStar();
	
	// Create planets
	sim::EntityHandle planet;
	sim::Scalar planet_radius = 10000000;
	planet = SpawnPlanet(sim::Sphere3(sim::Vector3::Zero(), planet_radius), 3634, 0);
	
	// Give formations time to expand.
	_applet_interface->Sleep(2);

	// Create observer.
	sim::EntityHandle observer = SpawnObserver(observer_start_pos);

	// Create origin controller.
	applet_interface.Launch("MonitorOrigin", 8192, &MonitorOrigin);
	
	// launch regulator
	applet_interface.Launch("Regulator", 8192, [] (applet::AppletInterface & applet_interface) {
		script::RegulatorScript regulator;
		regulator(applet_interface);
	});
	
	gfx::ObjectHandle skybox = SpawnSkybox();
	
	SpawnAnimats(applet_interface);

	// main loop
	while (! _applet_interface->GetQuitFlag())
	{
		applet_interface.WaitFor([& applet_interface] () {
			return ! _event_watcher.IsEmpty() || applet_interface.GetQuitFlag();
		});

		if (! HandleEvents(observer))
		{
			break;
		}
	}
	
	sun.Destroy();
	planet.Destroy();
	
	// remove skybox
	skybox.Destroy();

	observer.Destroy();
	
	ASSERT(_applet_interface == & applet_interface);
}
