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

#include "form/Scene.h"
#include "form/Manager.h"

#include "gfx/DebugGraphics.h"

#include "core/ConfigEntry.h"
#include "core/Matrix4.h"
#include "core/Vector2.h"

#include <sstream>
#include <fstream>


namespace ANONYMOUS {

//////////////////////////////////////////////////////////////////////
// functions

sim::Vector3 const default_camera_pos(0,99479.8,0);
CONFIG_DEFINE (use_default_camera_pos, bool, true);
CONFIG_DEFINE (camera_pos, sim::Vector3, default_camera_pos);
CONFIG_DEFINE (camera_rot, sim::Matrix4, static_cast<sim::Matrix4>(sim::Matrix4::Identity()));

CONFIG_DEFINE (planet_pos_1, sim::Vector3, sim::Vector3::Zero());
CONFIG_DEFINE (planet_radius_1, float, 100000);

// please don't write in
CONFIG_DEFINE (sun_orbit_distance, float, 100000000);	
CONFIG_DEFINE (sun_year, float, 300.f);
	
CONFIG_DEFINE (target_tick_proportion, double, .75f);

}


sim::Simulation::Simulation()
: observer(new Observer)
, formation_manager(new form::Manager(* observer))
//, focus(true)
, paused(false)
, capture(false)
, capture_frame(0)
, max_consecutive_frame_lag(10)
, consecutive_frame_lag(-1)
, fps(0)
, frame_count(0)
{
	frame_count_reset_time = app::GetTime();

	InitUniverse();
	
	gfx::DebugGraphics::Init();
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
	observer->SetRotation(camera_rot);
	
	Universe::Init();	
	Universe::AddEntity(* observer);
	//scene.AddEntity(* observer);
	scene.AddLight(observer->GetLight());
	
	Planet * planet = new Planet (planet_pos_1, planet_radius_1, 8);
	Universe::AddEntity(* planet);
	scene.AddEntity(* planet);
	
	Star * sun = new Star(sun_orbit_distance, sun_year);
	scene.AddLight(sun->GetLight());
	Universe::AddEntity(* sun);
}

sim::Simulation::~Simulation()
{
	gfx::DebugGraphics::Deinit();
	
	camera_pos = observer->GetPosition();
	camera_rot = observer->GetRotation();
	
	delete formation_manager;
	
	Universe::Deinit();
}

void sim::Simulation::Run()
{
	formation_manager->Launch();

	app::TimeType time = app::GetTime();
	app::TimeType target_tick_time = time;
	
	while (HandleEvents())
	{
		app::TimeType time_until_tick = target_tick_time - time;

		// Is it time for the next tick?
		if (time_until_tick <= 0) 
		{
			Tick();

			// Update time and calculate how long Tick() took. 
			app::TimeType tick_start_time = time;
			time = app::GetTime();
			app::TimeType tick_end_time = time;
			app::TimeType tick_seconds = tick_end_time - tick_start_time;

			// Adjust the number of nodes based on how well we're doing. 
			app::TimeType target_tick_seconds = Universe::target_frame_seconds * target_tick_proportion;
			formation_manager->AdjustNumNodes(tick_seconds, target_tick_seconds);

			// Set the next tick as a uniform period ahead. 
			target_tick_time += Universe::target_frame_seconds;

			continue;
		}
		
		if (formation_manager->PollMesh())
		{
			time = app::GetTime();
			continue;
		}
		
		//printf("sleep =%d %d\n", time, time_until_tick);
		app::Sleep(time_until_tick);
		time = app::GetTime();
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

void sim::Simulation::Render()
{
	PrintStats();

	Matrix4 rot = observer->GetRotation();
	Vector3 pos = observer->GetPosition();
	scene.SetCamera(pos, rot);

	renderer.Render(scene);
	
	SDL_GL_SwapBuffers();

	if (capture)
	{
		Capture();
	}

	++ frame_count;
	app::TimeType time = app::GetTime();
	app::TimeType delta = time - frame_count_reset_time;
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

	if (gfx::DebugGraphics::GetVerbosity() > .1) {
		gfx::DebugGraphics::out << "fps:" << 1. / fps << '\n';
	}
	
/*	if (DebugGraphics::GetVerbosity() > .4) {
		RealTimeFrequencyEstimator const & tps = formation_manager.GetTicksPerSecond();
		DebugGraphics::out << "tps:" << tps.GetFrequency() << '\n';
	}*/

/*	if (DebugGraphics::GetVerbosity() > .6) {
		DebugGraphics::out << "idle:" << idle_tick_period << '\n';
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
			if (app::IsKeyDown(KEY_LALT) || app::IsKeyDown(KEY_RALT))
			{
			}
			else {
				paused = ! paused;
			}
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
			
		case KEY_M:
			renderer.ToggleShadowMapping();
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
	
	return true;
}

