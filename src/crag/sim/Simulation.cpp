/*
 *  Simulation.cpp
 *  Crag
 *
 *  Created by John on 10/19/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Simulation.h"

#include "Firmament.h"
#include "Observer.h"
#include "Planet.h"
#include "Star.h"
#include "Universe.h"
#include "UserInput.h"

#include "physics/Body.h"
#include "physics/Singleton.h"

#include "form/scene/Scene.h"
#include "form/Manager.h"

#include "gfx/Debug.h"
#include "gfx/Renderer.h"

#include "core/ConfigEntry.h"
#include "geom/Matrix4.h"
#include "geom/Vector2.h"

#include <sstream>
#include <fstream>


using sys::TimeType;
using sys::GetTime;


namespace 
{

	//////////////////////////////////////////////////////////////////////
	// functions

	sim::Vector3 const default_camera_pos(0,9997750,0);
	CONFIG_DEFINE (use_default_camera_pos, bool, true);
	CONFIG_DEFINE (camera_pos, sim::Vector3, default_camera_pos);
	CONFIG_DEFINE (camera_rot, sim::Matrix4, static_cast<sim::Matrix4>(sim::Matrix4::Identity()));

	CONFIG_DEFINE (planet_pos, sim::Vector3, sim::Vector3::Zero());
	CONFIG_DEFINE (planet_radius_mean, sim::Scalar,  10000000);
	
	CONFIG_DEFINE (moon_pos, sim::Vector3, sim::Vector3(planet_radius_mean * 1.5, planet_radius_mean * 2.5, planet_radius_mean * 1.));
	//CONFIG_DEFINE (moon_pos, sim::Vector3, sim::Vector3(planet_radius_medium * .75, planet_radius_medium * 1.25, planet_radius_medium * .5));
	CONFIG_DEFINE (moon_radius_mean, sim::Scalar, 1500000);
	
	// please don't write in
	CONFIG_DEFINE (sun_orbit_distance, sim::Scalar, 100000000);	
	CONFIG_DEFINE (sun_year, sim::Scalar, 30000);
	
	CONFIG_DEFINE (target_work_proportion, double, .95f);
	CONFIG_DEFINE (startup_grace_period, TimeType, 2.f);

}


sim::Simulation::Simulation(bool init_enable_vsync)
: observer(new Observer)
, formation_manager(new form::Manager(* observer))
, enable_vsync(init_enable_vsync)
, paused(false)
, capture(false)
, capture_frame(0)
, start_time(GetTime())
{
	InitUniverse();
	
	gfx::Debug::Init();
}

void sim::Simulation::InitUniverse()
{
	scene.SetResolution(sys::GetWindowSize());
	scene.SetSkybox(new Firmament);
	
	if (use_default_camera_pos) {
		camera_pos = default_camera_pos;
		camera_rot = Matrix4::Identity();
	}
	observer->SetPosition(camera_pos);
	observer->GetBody()->SetRotation(camera_rot);
	
	sim::Universe & universe = sim::Universe::Get();
	universe.AddEntity(* observer);
	//scene.AddEntity(* observer);
	scene.AddLight(observer->GetLight());
	
	Planet * planet = new Planet (0, 8, planet_pos, planet_radius_mean);
	universe.AddEntity(* planet);
	scene.AddEntity(* planet);
	
	Planet * moon = new Planet (250, 10, moon_pos, moon_radius_mean);
	universe.AddEntity(* moon);
	scene.AddEntity(* moon);
	
	Star * sun = new Star(sun_orbit_distance, sun_year);
	scene.AddLight(sun->GetLight());
	universe.AddEntity(* sun);
}

sim::Simulation::~Simulation()
{
	gfx::Debug::Deinit();
	
	camera_pos = observer->GetPosition();
	observer->GetBody()->GetRotation(camera_rot);
	
	delete formation_manager;
}

void sim::Simulation::Run()
{
	formation_manager->Launch();
	
	TimeType next_tick_time = GetTime();
	
	while (HandleEvents())
	{
		TimeType time = GetTime();
		TimeType time_to_next_time = next_tick_time - time;
		if (time_to_next_time > 0)
		{
			Render();
		}
		else
		{
			Tick();
			
			if (time_to_next_time > 1)
			{
				next_tick_time = time;
			}
			else
			{
				next_tick_time += Universe::target_frame_seconds;
			}
		}
	}
	
	delete formation_manager;
	formation_manager = nullptr;
	
	DUMP_OBJECT(* formation_manager, std::cout);
}

void sim::Simulation::Tick()
{
	// Tick the entities.
	if (! paused && GetTime() > start_time + startup_grace_period) 
	{
		// Camera input.
		if (sys::HasFocus()) 
		{
			UserInput ui;
			Controller::Impulse impulse = ui.GetImpulse();
			observer->UpdateInput(impulse);
		}
		
		sim::Universe & universe = sim::Universe::Get();
		universe.Tick();
	}
	
	formation_manager->Tick();
}

void sim::Simulation::Render()
{
	// Now that the mesh is not needed, it's a good time to look at copying a new one.
	formation_manager->PollMesh();
	
	PrintStats();

	// Set scene's camera position from observer.
	physics::Body & observer_body = ref(observer->GetBody());
	Vector3 pos = observer_body.GetPosition();
	Matrix4 rot;
	observer_body.GetRotation(rot);
	scene.SetCamera(pos, rot);

	// Render scene and get an estimation of our load on system.
	TimeType frame_time = renderer.Render(scene, enable_vsync);
	
	// Regulator feedback.
	TimeType target_frame_time = Universe::target_frame_seconds;
	if (enable_vsync)
	{
		target_frame_time *= target_work_proportion;
	}
	formation_manager->SampleFrameRatio(frame_time, target_frame_time);
	
	// Screen capture.
	if (capture)
	{
		Capture();
	}
}

void sim::Simulation::PrintStats() const
{
	if (capture) {
		return;
	}
	
/*	if (Debug::GetVerbosity() > .4) {
		RealTimeFrequencyEstimator const & tps = formation_manager.GetTicksPerSecond();
		Debug::out << "tps:" << tps.GetFrequency() << '\n';
	}*/

/*	if (Debug::GetVerbosity() > .6) {
		Debug::out << "idle:" << idle_tick_period << '\n';
	}*/
}

void sim::Simulation::Capture()
{
	// Standard string/stream classes: 1/10.
	std::ostringstream filename_stream;
	filename_stream << "../../../" << capture_frame << ".jpg";
	std::string filename_string = filename_stream.str();
	
	Assert(false);
	//gfx::Image image = window.Capture();
	//image.SaveToFile(filename_string);
	
	++ capture_frame;
}

bool sim::Simulation::HandleEvents()
{
	sys::Event event;
	while (sys::GetEvent(event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				return false;

			case SDL_VIDEORESIZE:
				scene.SetResolution(Vector2i(event.resize.w, event.resize.h));
				formation_manager->ResetRegulator();
				break;

			case SDL_KEYDOWN:
				if (! OnKeyPress(event.key.keysym.sym))
				{
					return false;
				}
				break;
				
			case SDL_ACTIVEEVENT:
				formation_manager->ResetRegulator();
				break;

			default:
				break;
		}
	}

	return true;
}

// returns false iff the program should quit.
bool sim::Simulation::OnKeyPress(sys::KeyCode key_code)
{
	enum ModifierCombo 
	{
		COMBO_NONE,
		COMBO_SHIFT,
		COMBO_CTRL,
		COMBO_CTRL_SHIFT,
		COMBO_ALT,
		COMBO_ALT_SHIFT,
		COMBO_ALT_CTRL,
		COMBO_ALT_CTRL_SHIFT,
	};

	int combo_map = COMBO_NONE;	
	if (sys::IsKeyDown(KEY_LSHIFT) || sys::IsKeyDown(KEY_RSHIFT))
	{
		combo_map |= COMBO_SHIFT;
	}
	if (sys::IsKeyDown(KEY_LCTRL) || sys::IsKeyDown(KEY_RCTRL))
	{
		combo_map |= COMBO_CTRL;
	}
	if (sys::IsKeyDown(KEY_LALT) || sys::IsKeyDown(KEY_RALT))
	{
		combo_map |= COMBO_ALT;
	}

	switch (combo_map) 
	{
		case COMBO_NONE:
		{
			if ((int)key_code >= (int)KEY_0 && key_code <= (int)KEY_9)
			{
				int num = key_code - KEY_0;
				if (num == 0) {
					num = 10;
				}
				
				observer->SetSpeedFactor(num);
				return true;
			}
			
			switch (key_code)
			{
				case KEY_ESCAPE:
					return false;
					
				case KEY_RETURN:
					paused = ! paused;
					return true;
					
				case KEY_C:
					renderer.ToggleCulling();
					return true;
					
				case KEY_F:
					//renderer.ToggleSmoothShading();
					formation_manager->ToggleFlatShaded();
					return true;
					
				case KEY_G:
				{
					sim::Universe & universe = sim::Universe::Get();
					universe.ToggleGravity();
				}	return true;
					
				case KEY_I:
					formation_manager->ToggleSceneThread();
					return true;
					
				case KEY_L:
					renderer.ToggleLighting();
					return true;
					
				case KEY_O:
					capture = ! capture;
					return true;
					
				case KEY_P:
					renderer.ToggleWireframe();
					return true;
					
				default:
					break;
			}
			break;
		}
			
		case COMBO_SHIFT:
		{
			switch (key_code)
			{
				case KEY_C:
				{
					physics::Singleton & physics = physics::Singleton::Get();
					physics.ToggleCollisions();
					return true;
				}
					
				case KEY_I:
					formation_manager->ToggleMeshGeneration();
					return true;
					
				default:
					break;
			}
			break;
		}
			
		case COMBO_CTRL:
		{
			switch (key_code)
			{
				case KEY_I:
					formation_manager->ToggleDynamicOrigin();
					return true;
					
				default:
					break;
			}
			break;
		}
			
		default:
			break;
	}
	
	return true;
}
