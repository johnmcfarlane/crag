//
//  MainScript.cpp
//  crag
//
//  Created by John McFarlane on 2013-02-15.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "TestScript.h"
#include "MonitorOrigin.h"
#include "RegulatorScript.h"

#include "entity/sim/AnimatController.h"
#include "entity/SpawnEntityFunctions.h"
#include "entity/SpawnSkybox.h"

#include "applet/Applet.h"
#include "applet/AppletInterface_Impl.h"

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "physics/SphericalBody.h"

#include "gfx/axes.h"
#include "gfx/Engine.h"
#include "gfx/object/Ball.h"
#include "gfx/SetCameraEvent.h"

#include "geom/origin.h"

#include "core/ConfigEntry.h"
#include "core/EventWatcher.h"
#include "core/Random.h"

CONFIG_DEFINE(num_animats, int, 1);
CONFIG_DECLARE(origin_dynamic_enable, bool);

using geom::Vector3f;

namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// types
	typedef std::vector<sim::EntityHandle> EntityVector;
		
	////////////////////////////////////////////////////////////////////////////////
	// setup variables
	
	geom::rel::Vector3 observer_start_pos(-1085203, 3473659, 9306913);
	geom::rel::Vector3 animat_start_pos(-1085203, 3473659, 9306923);
	
	////////////////////////////////////////////////////////////////////////////////
	// variables

	applet::AppletInterface * _applet_interface;
	core::EventWatcher _event_watcher;
	bool _enable_dynamic_origin = true;
	std::vector<sim::EntityHandle> animats;
	
	////////////////////////////////////////////////////////////////////////////////
	// functions
	
	sim::EntityHandle SpawnAnimat(const sim::Vector3 & position)
	{
		auto animat = sim::EntityHandle::CreateHandle();

		sim::Sphere3 sphere(position, 1);
		animat.Call([sphere] (sim::Entity & entity) 
		{
			sim::Engine & engine = entity.GetEngine();
			physics::Engine & physics_engine = engine.GetPhysicsEngine();

			// physics
			auto zero_vector = sim::Vector3::Zero();
			auto & body = * new physics::SphericalBody(sim::Transformation(sphere.center), & zero_vector, physics_engine, sphere.radius);
			body.SetDensity(1);
			body.SetLinearDamping(0.005f);
			body.SetAngularDamping(0.005f);
			entity.SetLocation(& body);

			// graphics
			gfx::Transformation local_transformation(sphere.center, gfx::Transformation::Matrix33::Identity(), sphere.radius);
			gfx::ObjectHandle model = gfx::BallHandle::CreateHandle(local_transformation, sphere.radius, gfx::Color4f::Green());
			entity.SetModel(model);

			// controller
			auto controller = new sim::AnimatController(entity, sphere.radius);
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
	bool HandleEvents()
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
		gfx::SetCameraEvent event;
		event.transformation = geom::abs::Transformation(geom::Cast<geom::abs::Scalar>(observer_start_pos));

		applet::Daemon::Broadcast(event);
	}
	
	// Create sun. 
	geom::abs::Sphere3 star_volume(geom::abs::Vector3(9.34e6, 37480, 3.54e6), 1000000.);
	star_volume.center = geom::Resized(star_volume.center, 100000000.);
	gfx::Color4f star_color(gfx::Color4f(.2f,.55f,.9f) * 7500000000000000.f);
	sim::EntityHandle sun = SpawnStar(star_volume, star_color);
	
	// Create planets
	sim::EntityHandle planet;
	sim::Scalar planet_radius = 9999840;
	planet = SpawnPlanet(sim::Sphere3(sim::Vector3::Zero(), planet_radius), 3635, 0);
	
	// Create observer.
	sim::EntityHandle observer = SpawnObserver(observer_start_pos);

	// Create origin controller.
	if (origin_dynamic_enable)
	{
		applet_interface.Launch("MonitorOrigin", 8192, &MonitorOrigin);
	}
	
	// launch regulator
	applet_interface.Launch("Regulator", 8192, &RegulatorScript);
	
	gfx::ObjectHandle skybox = SpawnBitmapSkybox({{
		"assets/skybox/left.bmp",
		"assets/skybox/right.bmp",
		"assets/skybox/bottom.bmp",
		"assets/skybox/top.bmp",
		"assets/skybox/back.bmp",
		"assets/skybox/front.bmp"
	}});
	
	SpawnAnimats(animat_start_pos);

	// Give formations time to expand.
	_applet_interface->Sleep(.1f);

	// main loop
	while (! _applet_interface->GetQuitFlag())
	{
		applet_interface.WaitFor([& applet_interface] () {
			return ! _event_watcher.IsEmpty() || applet_interface.GetQuitFlag();
		});

		if (! HandleEvents())
		{
			break;
		}
	}
	
	sun.Destroy();
	planet.Destroy();
	
	// remove skybox
	skybox.Destroy();

	observer.Destroy();

	sim::Daemon::Call([] (sim::Engine & engine) 
	{
		engine.ForEachObject_DestroyIf([] (sim::Entity &) 
		{
			return true;
		});
	});
	
	ASSERT(_applet_interface == & applet_interface);
}
