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

#include "ga/AnimatController.h"

#include "applet/Applet.h"
#include "applet/AppletInterface_Impl.h"

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "physics/SphericalBody.h"

#include "gfx/axes.h"
#include "gfx/Engine.h"
#include "gfx/object/Ball.h"
#include "gfx/object/Skybox.h"
#include "gfx/SetCameraEvent.h"

#include "geom/origin.h"

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
	constexpr auto num_animats = 10;
	std::vector<sim::EntityHandle> animats(num_animats);
	
	////////////////////////////////////////////////////////////////////////////////
	// functions
	
	void DrawHolodeckSkybox(gfx::Skybox & skybox, int box_edge_size, int num_bars)
	{
		gfx::Image image;
		image.Create(geom::Vector2i(box_edge_size, box_edge_size));
		image.Clear(gfx::Color4b::Black());
		auto square_size = box_edge_size / num_bars;
		auto color = gfx::Color4f::Cyan();

		geom::Vector2i position, alt_position;
		for (position.x = 0; position.x < box_edge_size; position.x += square_size)
		{
			alt_position.x = position.x + square_size - 1;
			for (position.y = 0; position.y < box_edge_size; ++ position.y)
			{
				alt_position.y = position.y;
				image.SetPixel(position, color);
				image.SetPixel(alt_position, color);
				image.SetPixel(geom::Vector2i(position.y, position.x), color);
				image.SetPixel(geom::Vector2i(alt_position.y, alt_position.x), color);
			}
		}
	
		for (int axis = 0; axis < 3; ++ axis)
		{
			for (int pole = 0; pole < 2; ++ pole)
			{
				skybox.SetSide(axis, pole, image);
			}
		}
	}

	gfx::ObjectHandle SpawnHolodeckSkybox()
	{
		auto skybox = gfx::SkyboxHandle::CreateHandle();
		skybox.Call([] (gfx::Skybox & skybox) {
			DrawHolodeckSkybox(skybox, 512, 16);
		});

		return skybox;
	}

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
		for (auto& animat : animats)
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
		sim::Matrix33 rotation = gfx::Rotation(Vector3f(0, 1, 0), Vector3f(1, 0, 0));
		gfx::SetCameraEvent event;
		event.transformation = sim::Transformation(geom::Cast<sim::Scalar>(observer_start_pos), rotation);
		applet::Daemon::Broadcast(event);
	}
	
	// Create sun. 
	sim::EntityHandle sun = SpawnStar();
	
	// Create planets
	sim::EntityHandle planet;
	sim::Scalar planet_radius = 9999840;
	planet = SpawnPlanet(sim::Sphere3(sim::Vector3::Zero(), planet_radius), 3635, 0);
	
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
	
	gfx::ObjectHandle skybox = SpawnHolodeckSkybox();
	
	SpawnAnimats(observer_start_pos);

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
