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

#include "axes.h"
#include "Firmament.h"
#include "Observer.h"
#include "Star.h"

#include "physics/Singleton.h"

#include "form/scene/SceneThread.h"
#include "form/Manager.h"

#include "core/ConfigEntry.h"

#include "vm/Singleton.h"


using sys::TimeType;
using sys::GetTime;


namespace 
{

	//////////////////////////////////////////////////////////////////////
	// config variables

	// TODO: Currently not used. 
	// Find a way to reflect config variables in python. 
#if 0
	sim::Vector3 const default_camera_pos(0,9997750,0);
	CONFIG_DEFINE (use_default_camera_pos, bool, true);
	CONFIG_DEFINE (camera_pos, sim::Vector3, default_camera_pos);
	CONFIG_DEFINE (camera_rot, sim::Matrix4, static_cast<sim::Matrix4>(sim::Matrix4::Identity()));

	// please don't write in
	CONFIG_DEFINE (sun_orbit_distance, sim::Scalar, 100000000);	
	CONFIG_DEFINE (sun_year, sim::Scalar, 30000);
#endif
	
	CONFIG_DEFINE (target_work_proportion, double, .95f);
	CONFIG_DEFINE (startup_grace_period, TimeType, 1.f);
	
}


sim::Simulation::Simulation(bool init_enable_vsync)
: enable_vsync(init_enable_vsync)
, paused(false)
, capture(false)
, capture_frame(0)
, start_time(GetTime())
{
	SimulationPtr s(GetPtr());
	
	scene.SetResolution(sys::GetWindowSize());
	scene.SetSkybox(new Firmament);
	
	formation_manager = new form::Manager();
	
	SetCameraPos(Vector3::Zero(), Matrix4::Identity());
}

sim::Simulation::~Simulation()
{
	delete formation_manager;
}

sim::Universe const & sim::Simulation::GetUniverse() const
{
	return universe;
}

gfx::Scene & sim::Simulation::GetScene()
{
	return scene;
}

gfx::Scene const & sim::Simulation::GetScene() const
{
	return scene;
}

void sim::Simulation::AddEntity(Entity & entity)
{
	universe.AddEntity(entity);
	scene.AddEntity(entity);
}

void sim::Simulation::RemoveEntity(Entity & entity)
{
	universe.RemoveEntity(entity);
	scene.RemoveEntity(entity);
}

void sim::Simulation::SetCameraPos(Vector3 const & pos, Matrix4 const & rot)
{
	scene.SetCamera(pos, rot);
	formation_manager->GetSceneThread().SetCameraPos(axes::GetCameraRay(pos, rot));
}

void sim::Simulation::Run()
{
	vm::Singleton vm_singleton("./script/main.py");
	formation_manager->GetSceneThread().Launch();
	
	TimeType next_tick_time = GetTime();
	
	while (true)
	{
		if (! HandleEvents())
		{
			break;
		}
		
		if (vm::Singleton::Get().IsDone())
		{
			break;
		}
		
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
	SimulationPtr lock = GetPtr();

	// Tick the entities.
	if (! paused) 
	{
		bool physics = GetTime() > start_time + startup_grace_period;
		universe.Tick(physics);
	}
	
	formation_manager->Tick();
}

void sim::Simulation::Render()
{
	// Now that the mesh is not needed, it's a good time to look at copying a new one.
	formation_manager->PollMesh();
	
	PrintStats();

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
#if 0
	// Standard string/stream classes: 1/10.
	std::ostringstream filename_stream;
	filename_stream << "../../../" << capture_frame << ".jpg";
	std::string filename_string = filename_stream.str();
	
	Assert(false);
	//gfx::Image image = window.Capture();
	//image.SaveToFile(filename_string);
	
	++ capture_frame;
#endif
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
				formation_manager->GetSceneThread().ResetRegulator();
				break;

			case SDL_KEYDOWN:
				if (! OnKeyPress(event.key.keysym.sym))
				{
					return false;
				}
				break;
				
			case SDL_ACTIVEEVENT:
				formation_manager->GetSceneThread().ResetRegulator();
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
				
				Assert(false);
				//observer->SetSpeedFactor(num);
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
					formation_manager->GetSceneThread().ToggleFlatShaded();
					return true;
					
				case KEY_G:
				{
					universe.ToggleGravity();
				}	return true;
					
				case KEY_I:
					formation_manager->GetSceneThread().ToggleSuspended();
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
