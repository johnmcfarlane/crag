//
//  GameScript.cpp
//  crag
//
//  Created by John McFarlane on 2014-01-14.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "MonitorOrigin.h"
#include "RegulatorScript.h"

#include "entity/SpawnEntityFunctions.h"
#include "entity/SpawnSkybox.h"

#include "applet/Applet.h"
#include "applet/AppletInterface_Impl.h"

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "physics/Location.h"

#include "gfx/axes.h"
#include "gfx/Color.h"
#include "gfx/Debug.h"
#include "gfx/Engine.h"
#include "gfx/object/Object.h"
#include "gfx/SetCameraEvent.h"

#include "geom/origin.h"

#include "core/ConfigEntry.h"
#include "core/EventWatcher.h"
#include "core/Random.h"

using geom::Vector3f;

CONFIG_DECLARE(origin_dynamic_enable, bool);

namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// types
	typedef std::vector<sim::EntityHandle> EntityVector;
		
	////////////////////////////////////////////////////////////////////////////////
	// setup variables
	
	sim::Vector3 ufo_start_pos(0, 9999400, 0);
	sim::Vector3 camera_start_pos(10, 9999400, 0);
	size_t max_shapes = 50;
	bool cleanup_shapes = true;
	
	////////////////////////////////////////////////////////////////////////////////
	// variables

	Random random_sequence;
	applet::AppletInterface * _applet_interface;
	sim::EntityHandle _ufo;
	EntityVector _shapes;
	core::EventWatcher _event_watcher;
	bool _enable_dynamic_origin = true;
	
	////////////////////////////////////////////////////////////////////////////////
	// functions
	
	// random number generation
	double GetRandomUnit()
	{
		return random_sequence.GetUnit<double>();
	}
	
	void SpawnShapes(int shape_num)
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
				first->Destroy();
				_shapes.erase(first);
			}
		}
	
		if (_shapes.size() >= max_shapes)
		{
			return;
		}

		auto camera_ray = _applet_interface->Get<sim::Engine, sim::Ray3>([] (sim::Engine & engine) -> sim::Ray3 {
			return engine.GetCamera();
		});
	
		sim::Vector3 spawn_pos = geom::Project(camera_ray, sim::Scalar(5));
	
		gfx::Color4f color(Random::sequence.GetUnitInclusive<float>(), 
					Random::sequence.GetUnitInclusive<float>(), 
					Random::sequence.GetUnitInclusive<float>());

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
				
				case SDL_SCANCODE_COMMA:
					SpawnShapes(0);
					break;
				
				case SDL_SCANCODE_PERIOD:
					SpawnShapes(1);
					break;
				
				default:
					break;
			}
		}

		return true;
	}
}

// main entry point
void GameScript(applet::AppletInterface & applet_interface)
{
	FUNCTION_NO_REENTRY;

	_applet_interface = & applet_interface;
	
	// Create sun. 
	geom::abs::Sphere3 star_volume(geom::abs::Vector3(65062512., 75939904., 0.), 1000000.);
	gfx::Color4f star_color(gfx::Color4f(1.f,.95f,.85f) * 8000000000000000.f);
	sim::EntityHandle sun = SpawnStar(star_volume, star_color);
	
	// Set camera position
	{
		gfx::SetCameraEvent event;
		event.transformation.SetTranslation(geom::Cast<geom::abs::Scalar>(camera_start_pos));
		event.transformation.SetRotation(gfx::Rotation(geom::abs::Vector3(0, 0, 1)));
		gfx::Daemon::Broadcast(event);
	}
	
	// Create planets
	sim::EntityHandle planet;
	planet = SpawnPlanet(sim::Sphere3(sim::Vector3::Zero(), 10000000), 3634, 0);
	
	// Give formations time to expand.
	//_applet_interface->Sleep(2);	// (Currently doesn't work out so well.)

	// Create origin controller.
	if (origin_dynamic_enable)
	{
		applet_interface.Launch("MonitorOrigin", 8192, &MonitorOrigin);
	}
	
	// launch regulator
	applet_interface.Launch("Regulator", 8192, & RegulatorScript);
	
	gfx::ObjectHandle skybox = SpawnStarfieldSkybox();
	
	// Create ufo.
	_ufo = SpawnUfo(ufo_start_pos);

	// Create camera.
	sim::EntityHandle camera = SpawnCamera(camera_start_pos, _ufo);

	// main loop
	while (! _applet_interface->GetQuitFlag())
	{
		applet_interface.WaitFor([& applet_interface] () 
		{
			return ! _event_watcher.IsEmpty() || applet_interface.GetQuitFlag();
		});

		if (! HandleEvents())
		{
			break;
		}
	}
	
	while (! _shapes.empty())
	{
		_shapes.back().Destroy();
		_shapes.pop_back();
	}
	
	_ufo.Destroy();
	sun.Destroy();
	planet.Destroy();
	
	// remove skybox
	skybox.Destroy();

	camera.Destroy();
	
	ASSERT(_applet_interface == & applet_interface);
}
