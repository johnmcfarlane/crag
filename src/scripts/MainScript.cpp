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

#include <geom/Space.h>

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
	
	constexpr auto origin = geom::uni::Vector3(-1085436, 3474334, 9308749);
	constexpr auto observer_start_pos = geom::uni::Vector3(-1085436, 3474314, 9308759);
	constexpr auto animat_start_pos = geom::uni::Vector3(-1085436, 3474354, 9308749);

	constexpr auto star_volume_radius = geom::uni::Scalar(1000000.);
	constexpr auto star_volume_distance = geom::uni::Scalar(100000000.);
	auto star_volume_center = geom::Resized(geom::uni::Vector3 (.34e6f, 137480.f, .54e6f), star_volume_distance);
	constexpr auto star_color = gfx::Color4f(1.f,.975f,.95f);
	constexpr auto star_brightness = 7500000000000000.f;
	constexpr auto star_illumination = star_color * star_brightness;

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
	auto star_volume = geom::uni::Sphere3(star_volume_center, star_volume_radius);
	sim::EntityHandle sun = SpawnStar(star_volume, star_illumination);
	
	// Create planets
	sim::EntityHandle planet;
	sim::Scalar planet_radius = 9999840;
	planet = SpawnPlanet(sim::Sphere3(sim::Vector3::Zero(), planet_radius), 3635, 0);

	geom::Space space(origin);
	InitSpace(applet_interface, space);
	
	// Create observer.
	auto observer_forward = geom::Normalized(static_cast<sim::Vector3>(animat_start_pos - observer_start_pos));
	auto player_and_camera = SpawnPlayer(space.AbsToRel(observer_start_pos), observer_forward, space);
	
	gfx::ObjectHandle skybox = SpawnStarfieldSkybox();

	sim::Daemon::Call([] (sim::Engine & engine) {
		ssga::Init(engine, animat_start_pos);
	});

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
