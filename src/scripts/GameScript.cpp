//
//  GameScript.cpp
//  crag
//
//  Created by John McFarlane on 2014-01-14.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "InitSpace.h"

#include "entity/SpawnEntityFunctions.h"
#include "entity/SpawnSkybox.h"
#include "entity/SpawnPlayer.h"

#include "applet/AppletInterface_Impl.h"

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "gfx/Color.h"
#include "gfx/Engine.h"
#include "gfx/object/Object.h"

#include "core/ConfigEntry.h"
#include "core/EventWatcher.h"
#include "core/Random.h"

using geom::Vector3f;
using applet::AppletInterface;

CONFIG_DEFINE(profile_mode, false);

namespace
{
	////////////////////////////////////////////////////////////////////////////////
	// types
	typedef std::vector<sim::EntityHandle> EntityVector;
		
	////////////////////////////////////////////////////////////////////////////////
	// setup variables
	
	CONFIG_DEFINE(enable_spawn_ball, true);
	CONFIG_DEFINE(enable_spawn_cube, true);
	CONFIG_DEFINE(enable_spawn_obelisk, true);

#if defined(CRAG_OS_ANDROID)
	constexpr auto test_suspend_resume = false;
#else
	CONFIG_DEFINE(test_suspend_resume, false);
#endif

	CONFIG_DEFINE(profile_max_ticks, std::uint64_t(10000));

	geom::uni::Vector3 player_start_pos(5, 9998320, 0);
	size_t max_shapes = 250;
	bool cleanup_shapes = true;
	
	////////////////////////////////////////////////////////////////////////////////
	// variables

	Random random_sequence;
	EntityVector _shapes;
	
	////////////////////////////////////////////////////////////////////////////////
	// functions
	
	// random number generation
	double GetRandomUnit()
	{
		return random_sequence.GetFloat<double>();
	}
	
	void SpawnShapes(int shape_num, AppletInterface & applet_interface)
	{
		if (max_shapes == 0)
		{
			return;
		}
	
		if (cleanup_shapes)
		{
			if (_shapes.size() >= max_shapes)
			{
				EntityVector::iterator first = _shapes.begin();
				first->Release();
				_shapes.erase(first);
			}
		}
	
		if (_shapes.size() >= max_shapes)
		{
			return;
		}

		auto camera_ray = applet_interface.Get<sim::Engine, sim::Ray3>([] (sim::Engine & engine) -> sim::Ray3 {
			return engine.GetCamera();
		});
	
		sim::Vector3 spawn_pos = geom::Project(camera_ray, sim::Scalar(5));
	
		gfx::Color4f color(
			Random::sequence.GetFloatInclusive<float>(),
			Random::sequence.GetFloatInclusive<float>(),
			Random::sequence.GetFloatInclusive<float>());

		switch (shape_num)
		{
			case 0:
			{
				// ball
				float radius = geom::rel::Scalar(std::exp(- GetRandomUnit() * 2));
				sim::Sphere3 sphere(spawn_pos, radius);
				sim::EntityHandle ball = SpawnBall(sphere, sim::Vector3::Zero(), color);
				_shapes.push_back(ball);
				break;
			}
			
			case 1:
			{
				// box
				sim::Vector3 size(geom::rel::Scalar(std::exp(GetRandomUnit() * -1.)),
					geom::rel::Scalar(std::exp(GetRandomUnit() * -1.)),
					geom::rel::Scalar(std::exp(GetRandomUnit() * -1.)));
				sim::EntityHandle box = SpawnBox(spawn_pos, sim::Vector3::Zero(), size, color);
				_shapes.push_back(box);
				break;
			}
			
			default:
				ASSERT(false);
		}
	}

	// returns true if the applet should NOT quit
	void HandleEvents(core::EventWatcher & event_watcher, AppletInterface & applet_interface)
	{
		int num_events = 0;
	
		SDL_Event event;
		while (event_watcher.PopEvent(event))
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

				case SDL_SCANCODE_COMMA:
					SpawnShapes(0, applet_interface);
					break;
				
				case SDL_SCANCODE_PERIOD:
					SpawnShapes(1, applet_interface);
					break;
				
				default:
					break;
			}
		}
	}

	void RandomlyStutter(AppletInterface & applet_interface)
	{
		bool suspend = Random::sequence.GetBool();
		DEBUG_MESSAGE("suspend=%d", int(suspend));
		gfx::Daemon::Call([suspend](gfx::Engine &engine)
		{
			engine.SetIsSuspended(suspend);
		});

		if (Random::sequence.GetBool())
		{
			float sleep = Squared(Random::sequence.GetFloat());
			DEBUG_MESSAGE("sleep=%f", sleep);
			applet_interface.WaitFor(sleep);
		}
	}
}

// main entry point
void GameScript(AppletInterface & applet_interface)
{
	FUNCTION_NO_REENTRY;

	core::EventWatcher event_watcher;

	// coordinate system
	geom::Space space(player_start_pos);
	auto rel_player_start_pos = space.AbsToRel(player_start_pos);
	
	// Create sun. 
	geom::uni::Sphere3 star_volume(geom::uni::Vector3(65062512., 75939904., 0.), 1000000.);
	gfx::Color4f star_color(gfx::Color4f(.6f,.8f,1.f) * 8000000000000000.f);
	sim::EntityHandle sun = SpawnStar(star_volume, star_color);
	
	// Create planets
	sim::EntityHandle planet;
	planet = SpawnPlanet(sim::Sphere3(sim::Vector3::Zero(), 10000000.f), 3634, 0);
	
	InitSpace(applet_interface, space);

	sim::Daemon::Call([] (sim::Engine & engine)
	{
		engine.IncrementPause(1);
	});

#if defined(CRAG_OS_ANDROID)
	gfx::ObjectHandle skybox = SpawnBitmapSkybox({{
		"assets/skybox/right.png",
		"assets/skybox/left.png",
		"assets/skybox/top.png",
		"assets/skybox/bottom.png",
		"assets/skybox/front.png",
		"assets/skybox/back.png"
	}});
#else
	gfx::ObjectHandle skybox = SpawnBitmapSkybox({{
		"assets/skybox/right.bmp",
		"assets/skybox/left.bmp",
		"assets/skybox/top.bmp",
		"assets/skybox/bottom.bmp",
		"assets/skybox/front.bmp",
		"assets/skybox/back.bmp"
	}});
#endif
	
	// Create player.
	auto player_and_camera = SpawnPlayer(rel_player_start_pos, sim::Vector3(0, 0, 1), space);

	// ball
	if (enable_spawn_ball)
	{
		auto sphere = sim::Sphere3(rel_player_start_pos + sim::Vector3(5.f, 8.f, -4.f), 1.f);
		auto ball = SpawnBall(sphere, sim::Vector3::Zero(), gfx::Color4f::Periwinkle());
		_shapes.push_back(ball);
	}

	// cube
	if (enable_spawn_cube)
	{
		auto spawn_pos = sim::Vector3(rel_player_start_pos + sim::Vector3(0.f, 10.f, 3.f));
		auto size = sim::Vector3(1.f, 1.f, 1.f) * 2.f;
		auto cube = SpawnBox(spawn_pos, sim::Vector3::Zero(), size, gfx::Color4f::Orange());
		_shapes.push_back(cube);
	}
	
	// obelisk
	if (enable_spawn_obelisk)
	{
		auto spawn_pos = sim::Vector3(rel_player_start_pos + sim::Vector3(1.f, 8.f, 3.f));
		auto size = sim::Vector3(1.f, 4.f, 9.f) * .5f;
		auto obelisk = SpawnBox(spawn_pos, sim::Vector3::Zero(), size, gfx::Color4f::Black());
		_shapes.push_back(obelisk);
	}

	std::uint64_t starting_num_ticks = applet_interface.Get<sim::Engine, std::uint64_t>([] (sim::Engine & engine) -> std::uint64_t {
		engine.IncrementPause(-1);
		return engine.GetNumTicks();
	});
	auto first_tick_time = app::GetTime();

	// main loop
	while (applet_interface.WaitFor(0))
	{
		if (profile_mode)
		{
			SpawnShapes(0, applet_interface);
			SpawnShapes(1, applet_interface);

			auto num_ticks = applet_interface.Get<sim::Engine, std::uint64_t>([] (sim::Engine & engine) -> std::uint64_t {
				return engine.GetNumTicks();
			}) - starting_num_ticks;
			if (num_ticks > profile_max_ticks)
			{
				auto last_tick_time = app::GetTime();
				ERROR_MESSAGE("Profile: %" PRIu64 " ticks in %lf seconds\n",
					num_ticks, last_tick_time - first_tick_time);
				app::Quit();
			}
		}

		HandleEvents(event_watcher, applet_interface);

		if (test_suspend_resume)
		{
			RandomlyStutter(applet_interface);
		}
	}
	
	while (! _shapes.empty())
	{
		_shapes.back().Release();
		_shapes.pop_back();
	}
	
	player_and_camera[1].Release();
	player_and_camera[0].Release();
	sun.Release();
	planet.Release();
	
	// remove skybox
	skybox.Release();

	// when this function quits, the program must quit
	ASSERT(applet_interface.GetQuitFlag());
}
