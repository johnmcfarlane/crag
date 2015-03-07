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

#include "entity/sim/AnimatController.h"
#include "entity/SpawnEntityFunctions.h"
#include "entity/SpawnPlayer.h"
#include "entity/SpawnSkybox.h"

#include "applet/Applet.h"

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "physics/SphereBody.h"

#include "gfx/Engine.h"
#include "gfx/object/Ball.h"

#include "core/ConfigEntry.h"
#include "core/EventWatcher.h"
#include "core/Random.h"

CONFIG_DECLARE(player_type, int);
CONFIG_DEFINE(num_animats, 1);
CONFIG_DECLARE(origin_dynamic_enable, bool);

using geom::Vector3f;
using applet::AppletInterface;

namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// types
	typedef std::vector<sim::EntityHandle> EntityVector;
		
	////////////////////////////////////////////////////////////////////////////////
	// setup variables
	
	geom::abs::Vector3 observer_start_pos(-1085203, 3473659, 9306913);
	geom::abs::Vector3 animat_start_pos(-1085203, 3473659, 9306923);
	
	////////////////////////////////////////////////////////////////////////////////
	// variables

	core::EventWatcher _event_watcher;
	bool _enable_dynamic_origin = true;
	std::vector<sim::EntityHandle> animats;
	
	////////////////////////////////////////////////////////////////////////////////
	// functions
	
	sim::EntityHandle SpawnAnimat(const sim::Vector3 & position)
	{
		auto animat = sim::EntityHandle::Create();

		sim::Sphere3 sphere(position, 1);
		animat.Call([sphere] (sim::Entity & entity) 
		{
			sim::Engine & engine = entity.GetEngine();
			physics::Engine & physics_engine = engine.GetPhysicsEngine();

			// physics
			auto zero_vector = sim::Vector3::Zero();
			auto body = std::make_shared<physics::SphereBody>(sim::Transformation(sphere.center), & zero_vector, physics_engine, sphere.radius);
			body->SetDensity(1);
			entity.SetLocation(body);

			// graphics
			gfx::Transformation local_transformation(sphere.center, gfx::Transformation::Matrix33::Identity(), sphere.radius);
			gfx::ObjectHandle model = gfx::BallHandle::Create(local_transformation, sphere.radius, gfx::Color4f::Green());
			entity.SetModel(model);

			// controller
			auto controller = std::make_shared<sim::AnimatController>(entity, sphere.radius);
			entity.SetController(controller);
		});

		return animat;
	}

	void SpawnAnimats(Vector3f base_position)
	{
		animats.resize(num_animats);
		for (auto & animat : animats)
		{
			Vector3f offset;
			float r;
			Random::sequence.GetGaussians(offset.x, offset.y);
			offset.y = std::abs(offset.y);
			Random::sequence.GetGaussians(offset.z, r);

			auto position = base_position + offset * 10.f;

			animat = SpawnAnimat(position);
		}
	}

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
	SpawnAnimats(rel_animat_start_pos);

	// main loop
	while (! applet_interface.Sleep(0))
	{
		HandleEvents();
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
