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

#include "entity/sim/AnimatController.h"

#include "applet/Applet.h"

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "gfx/Engine.h"
#include "gfx/object/Ball.h"

#include "core/ConfigEntry.h"
#include "core/EventWatcher.h"
#include "core/iterable_object_pool.h"
#include <core/Random.h>

CONFIG_DECLARE(player_type, int);
CONFIG_DEFINE(num_animats, 50);
CONFIG_DECLARE(origin_dynamic_enable, bool);
CONFIG_DEFINE(animat_birth_bump, 1000.f);

using geom::Vector3f;
using applet::AppletInterface;

namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// types
	typedef std::vector<sim::EntityHandle> EntityVector;
		
	////////////////////////////////////////////////////////////////////////////////
	// setup variables
	
	geom::abs::Vector3 observer_start_pos(-1085436, 3474334, 9308749);
	geom::abs::Vector3 animat_start_pos(-1085203, 3473659, 9306923);
	
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

	void Breed()
	{
		sim::Daemon::Call([] (sim::Engine & engine)
		{
			auto & pool = sim::AnimatController::GetPool();
			while (pool.size() < num_animats)
			{
				auto pool_size = pool.size();

				auto parent_genomes = std::array<sim::ga::Genome const *, 2>();
				auto sum_position = sim::Vector3::Zero();

				for (auto parent_index = 0; parent_index != 2; ++ parent_index)
				{
					// get controller
					auto pool_index = Random::sequence.GetInt(pool_size);

					auto parent_controller = static_cast<sim::AnimatController * const>(nullptr);
					pool.for_each([& pool_index, & parent_controller] (sim::AnimatController & controller) {
						if (! pool_index)
						{
							parent_controller = & controller;
						}

						-- pool_index;
					});
					CRAG_VERIFY_TRUE(parent_controller);

					// get genome
					parent_genomes[parent_index] = & parent_controller->GetGenome();

					// positional
					auto & location = * parent_controller->GetEntity().GetLocation();
					sum_position += location.GetTranslation();
				};

				if (parent_genomes[0] == parent_genomes[1])
				{
					continue;
				}

				auto child_genome = sim::ga::Genome(* parent_genomes[0], * parent_genomes[1]);
				auto child_horizontal_position = sum_position * .5f;

				auto handle = sim::EntityHandle::CreateFromUid(ipc::Uid::Create());
				auto entity = engine.CreateObject<sim::Entity>(handle);
				ConstructAnimat(* entity, child_horizontal_position, std::move(child_genome));
			}
		});
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
	
	auto rel_animat_start_pos = space.AbsToRel(animat_start_pos);
	SpawnAnimats(rel_animat_start_pos, num_animats);

	// main loop
	while (applet_interface.WaitFor(0))
	{
		HandleEvents();

		Breed();
	}
	
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
