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

#include "form/scene/Scene.h"
#include "form/Manager.h"

#include "gfx/Debug.h"

#include "core/ConfigEntry.h"
#include "geom/Matrix4.h"
#include "geom/Vector2.h"

#include <sstream>
#include <fstream>


using app::TimeType;


namespace 
{

	//////////////////////////////////////////////////////////////////////
	// functions

	sim::Vector3 const default_camera_pos(0,997740.6,0);
	CONFIG_DEFINE (use_default_camera_pos, bool, true);
	CONFIG_DEFINE (camera_pos, sim::Vector3, default_camera_pos);
	CONFIG_DEFINE (camera_rot, sim::Matrix4, static_cast<sim::Matrix4>(sim::Matrix4::Identity()));

	CONFIG_DEFINE (planet_pos_1, sim::Vector3, sim::Vector3::Zero());
	CONFIG_DEFINE (planet_min_radius_1, float,  999000);
	CONFIG_DEFINE (planet_max_radius_1, float, 1001000);

	// please don't write in
	CONFIG_DEFINE (sun_orbit_distance, float, 100000000);	
	CONFIG_DEFINE (sun_year, float, 300.f);
		
	CONFIG_DEFINE (target_work_proportion, double, .95f);

}


sim::Simulation::Simulation()
: observer(new Observer)
, formation_manager(new form::Manager(* observer))
, paused(false)
, capture(false)
, capture_frame(0)
, running_poll_seconds(0)
, fps(0)
, frame_count(0)
, cached_time(0)
{
	frame_count_reset_time = GetTime(true);

	InitUniverse();
	
	gfx::Debug::Init();
}

void sim::Simulation::InitUniverse()
{
	scene.SetResolution(app::GetWindowSize());
	scene.SetSkybox(new Firmament);	// TODO: Time for a ref-counter system.
	
	if (use_default_camera_pos) {
		camera_pos = default_camera_pos;
		camera_rot = Matrix4::Identity();
	}
	observer->SetPosition(camera_pos);
	observer->GetBody()->SetRotation(camera_rot);
	
	Universe::Init();	
	Universe::AddEntity(* observer);
	//scene.AddEntity(* observer);
	scene.AddLight(observer->GetLight());
	
	Planet * planet = new Planet (planet_pos_1, planet_min_radius_1, planet_max_radius_1, 8);
	Universe::AddEntity(* planet);
	scene.AddEntity(* planet);
	
	Star * sun = new Star(sun_orbit_distance, sun_year);
	scene.AddLight(sun->GetLight());
	Universe::AddEntity(* sun);
}

sim::Simulation::~Simulation()
{
	gfx::Debug::Deinit();
	
	camera_pos = observer->GetPosition();
	observer->GetBody()->GetRotation(camera_rot);
	
	delete formation_manager;
	
	Universe::Deinit();
}

void sim::Simulation::Run()
{
	formation_manager->Launch();

	TimeType const target_frame_seconds = Universe::target_frame_seconds;
	TimeType const target_work_seconds = target_frame_seconds * target_work_proportion;
	
	TimeType target_tick_time = GetTime(true);
	
	while (HandleEvents())
	{
		TimeType time_until_next_tick = target_tick_time - GetTime();

		// Is it time for the next tick?
		if (time_until_next_tick > 0) 
		{
			// If not, sleep until it is.
			app::Sleep(time_until_next_tick);
			GetTime(true);
			continue;
		}

		// Set the next tick as a uniform period ahead. 
		target_tick_time += target_frame_seconds;
		
		// Remember when tick started.
		TimeType tick_start_time = GetTime();
		
		Tick();

		// Calculate how long Tick() took. 
		TimeType tick_seconds = GetTime(true) - tick_start_time;
		
		// Don't let the buffer swap be part of the time as vsync can cause a delay.
		SDL_GL_SwapBuffers();

		TimeType pre_swap_time = GetTime(false);
		TimeType post_swap_time = GetTime(true);
		TimeType swap_seconds = post_swap_time - pre_swap_time;

		PollMesh();
		
		// Adjust the number of nodes based on how well we're doing. 
		// Take into account the tick duration AND the time it takes to poll a new mesh.
		TimeType work_seconds = tick_seconds + running_poll_seconds;
		formation_manager->AdjustNumNodes(work_seconds, target_work_seconds);
		
		if (gfx::Debug::GetVerbosity() > .65)
		{
			gfx::Debug::out << "tick_seconds:" << tick_seconds << '\n';
			gfx::Debug::out << "poll_seconds:" << running_poll_seconds << '\n';
			gfx::Debug::out << "work_seconds:" << work_seconds << '\n';
			gfx::Debug::out << "swap_seconds:" << swap_seconds << '\n';
			gfx::Debug::out << "target_work_seconds:" << target_work_seconds << '\n';
		}
	}
	
	delete formation_manager;
	formation_manager = nullptr;
	
	DUMP_OBJECT(* formation_manager, std::cout);
}

void sim::Simulation::Tick()
{
	// Camera input.
	if (app::HasFocus()) {
		UserInput ui;
		Controller::Impulse impulse = ui.GetImpulse();
		observer->UpdateInput(impulse);
	}

	// Tick the entities.
	if (! paused) {
		Universe::Tick();
	}
	
	formation_manager->Tick();

	Render();
}

bool sim::Simulation::PollMesh()
{
	// Poll for a new mesh.
	TimeType poll_start_time = GetTime();
	if (! formation_manager->PollMesh())
	{
		return false;
	}

	TimeType poll_seconds = GetTime(true) - poll_start_time;
	running_poll_seconds = Max(poll_seconds, running_poll_seconds * .95f);
	
	return true;
}

void sim::Simulation::Render()
{
	PrintStats();

	physics::Body & observer_body = ref(observer->GetBody());
	Vector3 pos = observer_body.GetPosition();

	Matrix4 rot;
	observer_body.GetRotation(rot);
	
	scene.SetCamera(pos, rot);

	renderer.Render(scene);

	// Note: The render still requires a buffer swap,
	// but as vsync is turned on, this could distort the timing results.

	if (capture)
	{
		Capture();
	}

	++ frame_count;
	TimeType time = app::GetTime();
	TimeType delta = time - frame_count_reset_time;
	if (delta > 1) 
	{
		frame_count_reset_time = time;
		fps = static_cast<double>(delta) / frame_count;
		frame_count = 0;
	}
}

void sim::Simulation::PrintStats() const
{
	if (capture) {
		return;
	}

	if (gfx::Debug::GetVerbosity() > .1) {
		gfx::Debug::out << "fps:" << 1. / fps << '\n';
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
	app::Event event;
	while (app::GetEvent(event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				return false;

			case SDL_VIDEORESIZE:
				scene.SetResolution(Vector2i(event.resize.w, event.resize.h));
				break;

			case SDL_KEYDOWN:
				if (! OnKeyPress(event.key.keysym.sym))
				{
					return false;
				}
				break;

			default:
				break;
		}
	}

	return true;
}

// returns false iff the program should quit.
bool sim::Simulation::OnKeyPress(app::KeyCode key_code)
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
	if (app::IsKeyDown(KEY_LSHIFT) || app::IsKeyDown(KEY_RSHIFT))
	{
		combo_map |= COMBO_SHIFT;
	}
	if (app::IsKeyDown(KEY_LCTRL) || app::IsKeyDown(KEY_RCTRL))
	{
		combo_map |= COMBO_CTRL;
	}
	if (app::IsKeyDown(KEY_LALT) || app::IsKeyDown(KEY_RALT))
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
					break;
					
				case KEY_C:
					renderer.ToggleCulling();
					break;
					
				case KEY_F:
					renderer.ToggleSmoothShading();
					break;
					
				case KEY_G:
					Universe::ToggleGravity();
					break;
					
				case KEY_I:
					formation_manager->ToggleSuspended();
					break;
					
				case KEY_L:
					renderer.ToggleLighting();
					break;
					
				case KEY_O:
					capture = ! capture;
					break;
					
				case KEY_P:
					renderer.ToggleWireframe();
					break;
					
				default:
					break;
			}
			break;
		}
	}
	
	return true;
}

TimeType sim::Simulation::GetTime(bool update_time)
{	
	if (update_time)
	{
		TimeType t = app::GetTime();
		Assert(t >= cached_time);
		cached_time = t;
	}
/*	else 
	{
#if ! defined(NDEBUG)
		TimeType t = app::GetTime();
		TimeType passed = t - cached_time;
		Assert(passed < 1.);
#endif
	}*/

	return cached_time;
}
