//
//  MainScript.cpp
//  crag
//
//  Created by John McFarlane on 2013-02-15.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "InitSpace.h"

#include "entity/SpawnEntityFunctions.h"
#include "entity/SpawnPlayer.h"
#include "entity/SpawnSkybox.h"

#include "entity/sim/ssga.h"

#include "applet/Applet.h"

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "gfx/Engine.h"
#include "gfx/object/Ball.h"

#include "core/ConfigEntry.h"
#include "core/EventWatcher.h"
#include "core/iterable_object_pool.h"

CONFIG_DECLARE(player_type, int);
CONFIG_DECLARE(origin_dynamic_enable, bool);

using applet::AppletInterface;

namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// types
	typedef std::vector<sim::EntityHandle> EntityVector;
		
	////////////////////////////////////////////////////////////////////////////////
	// setup variables
	
	geom::abs::Vector3 observer_start_pos(-1085436, 3474334, 9308749);

	////////////////////////////////////////////////////////////////////////////////
	// variables

	core::EventWatcher _event_watcher;

	////////////////////////////////////////////////////////////////////////////////
	// functions
	
	// returns true if the applet should NOT quit
	void HandleEvents()
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
					app::Quit();
					break;
				}

				default:
					break;
			}
		}
	}
}

// main entry point
void MainScript(AppletInterface & applet_interface)
{
	FUNCTION_NO_REENTRY;

	// Create sun. 
	geom::abs::Sphere3 star_volume(geom::abs::Vector3(9.34e6, 37480, 3.54e6), 1000000.);
	star_volume.center = geom::Resized(star_volume.center, 100000000.);
	gfx::Color4f star_color(gfx::Color4f(1.f,.975f,.95f) * 7500000000000000.f);
	sim::EntityHandle sun = SpawnStar(star_volume, star_color);
	
	// Create planets
	sim::EntityHandle planet;
	sim::Scalar planet_radius = 9999840;
	planet = SpawnPlanet(sim::Sphere3(sim::Vector3::Zero(), planet_radius), 3635, 0);

	geom::Space space(observer_start_pos);
	InitSpace(applet_interface, space);
	
	// Create observer.
	auto player_and_camera = SpawnPlayer(sim::Vector3::Zero(), space);
	
	gfx::ObjectHandle skybox = SpawnStarfieldSkybox();

	sim::Daemon::Call(& ssga::Init);

	// main loop
	while (applet_interface.WaitFor(0))
	{
		HandleEvents();

		sim::Daemon::Call(& ssga::Tick);
	}

	sim::Daemon::Call(& ssga::Deinit);

	sun.Release();
	planet.Release();
	
	// remove skybox
	skybox.Release();

	player_and_camera[1].Release();
	player_and_camera[0].Release();

	sim::Daemon::Call([] (sim::Engine & engine) 
	{
		engine.ForEachObject_ReleaseIf([] (sim::Entity &) 
		{
			return true;
		});
	});

	// when this function quits, the program must quit
	ASSERT(applet_interface.GetQuitFlag());
}
