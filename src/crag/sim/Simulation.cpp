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
#include "Entity.h"
#include "Universe.h"

#include "physics/Engine.h"

#include "form/scene/SceneThread.h"
#include "form/FormationManager.h"

#include "script/ScriptThread.h"

#include "gfx/Renderer.h"
#include "gfx/Scene.h"

#include "core/ConfigEntry.h"


namespace 
{

	//////////////////////////////////////////////////////////////////////
	// config variables

	CONFIG_DEFINE (target_work_proportion, double, .95f);
	
}


////////////////////////////////////////////////////////////////////////////////
// sim::Simulation

CONFIG_DEFINE_MEMBER (sim::Simulation, target_frame_seconds, double, 1.f / 60.f);

sim::Simulation::Simulation()
: quit_flag(false)
, paused(false)
, capture(false)
, capture_frame(0)
, universe(new Universe)
, physics_engine(new physics::Engine)
, scene(nullptr)
, renderer(nullptr)
{
	Assert(singleton == nullptr);
	singleton = this;
}

sim::Simulation::~Simulation()
{
	Assert(singleton == this);
	singleton = nullptr;

	delete universe;
	universe = nullptr;

	delete physics_engine;
	physics_engine = nullptr;
}

void sim::Simulation::OnMessage(smp::TerminateMessage const & message)
{
	quit_flag = true;
}

void sim::Simulation::OnMessage(AddEntityMessage const & message)
{
	Entity & entity = message.entity;
	if (! entity.Init(message.args))
	{
		std::cout << "Bad input parameters to entity\n";
		DEBUG_BREAK();
	}
	
	// Let go of arguments object.
	// (Incremented in script::MetaClass::NewObject.)
	Py_DECREF(& message.args);
	
	universe->AddEntity(entity);
	scene->AddEntity(entity);
}

void sim::Simulation::OnMessage(RemoveEntityMessage const & message)
{
	universe->RemoveEntity(message.entity);
	scene->RemoveEntity(message.entity);
	
	delete & message.entity;
}

void sim::Simulation::OnMessage(SetCameraMessage const & message)
{
	scene->SetCamera(message.projection.pos, message.projection.rot);

	form::FormationManager::SendMessage(message);
}

sys::TimeType sim::Simulation::GetTime() const
{
	return universe->GetTime();
}

sim::Universe & sim::Simulation::GetUniverse() 
{
	return ref(universe);
}

physics::Engine & sim::Simulation::GetPhysicsEngine()
{
	return ref(physics_engine);
}

gfx::Scene & sim::Simulation::GetScene()
{
	return ref(scene);
}

void sim::Simulation::Run()
{
	FUNCTION_NO_REENTRY;

	// init graphics stuff
	renderer = new gfx::Renderer;
	scene = new gfx::Scene;
	scene->SetResolution(sys::GetWindowSize());
	scene->SetSkybox(new Firmament);
	
	sys::TimeType next_tick_time = sys::GetTime();
	
	do
	{
		sys::TimeType time = sys::GetTime();
		sys::TimeType time_to_next_tick = next_tick_time - time;
		if (time_to_next_tick > 0)
		{
			Render();
		}
		else
		{
			Tick();
			
			if (time_to_next_tick > 1)
			{
				next_tick_time = time;
			}
			else
			{
				next_tick_time += target_frame_seconds;
			}
		}

		ProcessMessages();
	}
	while (! quit_flag);
	
	delete scene;
	scene = nullptr;

	//DUMP_OBJECT(* form_thread, std::cout);
	delete renderer;
	renderer = nullptr;
}

void sim::Simulation::Tick()
{
	// Tick the entities.
	if (! paused) 
	{
		physics_engine->Tick(target_frame_seconds);
		
		universe->Tick(target_frame_seconds);
	}
}

void sim::Simulation::Render()
{
	form::FormationManager & formation_manager = form::FormationManager::Ref();
	formation_manager.PollMesh();
	
	PrintStats();

	// Render scene and get an estimation of our load on system.
	sys::TimeType frame_time = renderer->Render(* scene);
	
	// Regulator feedback.
	sys::TimeType target_frame_time = target_frame_seconds;
	target_frame_time *= target_work_proportion;
	
	formation_manager.SampleFrameRatio(frame_time, target_frame_time);
	
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
		RealTimeFrequencyEstimator const & tps = form_thread.GetTicksPerSecond();
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

bool sim::Simulation::HandleEvent(sys::Event const & event)
{
	switch (event.type)
	{
		case SDL_VIDEORESIZE:
		{
			scene->SetResolution(Vector2i(event.resize.w, event.resize.h));
			form::FormationManager::Ref().ResetRegulator();
			return true;
		}

		case SDL_KEYDOWN:
			return OnKeyPress(event.key.keysym.sym);
			
		case SDL_ACTIVEEVENT:
		{
			form::FormationManager::Ref().ResetRegulator();			
			return true;
		}

		default:
			return false;
	}
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
			switch (key_code)
			{
				case SDLK_RETURN:
					paused = ! paused;
					return true;
					
				case SDLK_c:
					renderer->ToggleCulling();
					return true;
					
				case SDLK_f:
					form::FormationManager::Ref().ToggleFlatShaded();
					return true;
					
				case SDLK_g:
					universe->ToggleGravity();
					return true;
					
				case SDLK_i:
					form::FormationManager::Ref().ToggleSuspended();
					return true;
					
				case SDLK_l:
					renderer->ToggleLighting();
					return true;
					
				case SDLK_o:
					capture = ! capture;
					return true;
					
				case SDLK_p:
					renderer->ToggleWireframe();
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
				case SDLK_c:
				{
					physics_engine->ToggleCollisions();
					return true;
				}
					
				case SDLK_i:
					form::FormationManager::Ref().ToggleMeshGeneration();
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
				case SDLK_i:
					form::FormationManager::Ref().ToggleDynamicOrigin();
					return true;
					
				default:
					break;
			}
			break;
		}
			
		default:
			break;
	}
	
	return false;
}


sim::Simulation * sim::Simulation::singleton = nullptr;
