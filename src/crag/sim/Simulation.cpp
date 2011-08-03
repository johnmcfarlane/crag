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

#include "core/ConfigEntry.h"


////////////////////////////////////////////////////////////////////////////////
// sim::Simulation

CONFIG_DEFINE_MEMBER (sim::Simulation, target_frame_seconds, double, 1.f / 60.f);

sim::Simulation::Simulation()
: super(1024)
, quit_flag(false)
, paused(false)
, capture(false)
, capture_frame(0)
, universe(new Universe)
, physics_engine(new physics::Engine)
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
	entity.UpdateModels();
}

void sim::Simulation::OnMessage(RemoveEntityMessage const & message)
{
	universe->RemoveEntity(message.entity);
	
	delete & message.entity;
}

void sim::Simulation::OnMessage(gfx::RendererReadyMessage const & message)
{
	Assert(false);
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

void sim::Simulation::Run()
{
	FUNCTION_NO_REENTRY;
	
	smp::SetThreadPriority(0);
	smp::SetThreadName("Simulation");

	gfx::AddObjectMessage add_skybox_message = { ref(new Firmament) };
	gfx::Renderer::SendMessage(add_skybox_message);
	
	sys::TimeType next_tick_time = sys::GetTime();
	
	while (! quit_flag)
	{
		sys::TimeType time = sys::GetTime();
		sys::TimeType time_to_next_tick = next_tick_time - time;
		if (time_to_next_tick > 0)
		{
			if (ProcessMessages() == 0)
			{
				smp::Sleep(0);
			}
		}
		else
		{
			Tick();
			
			if (time_to_next_tick < -1)
			{
				next_tick_time = time;
			}
			else
			{
				next_tick_time += target_frame_seconds;
			}
		}
	}
	
	gfx::RemoveObjectMessage remove_skybox_message = { add_skybox_message._object };
	gfx::Renderer::SendMessage(remove_skybox_message);
}

void sim::Simulation::Tick()
{
	// Tick the entities.
	if (! paused) 
	{
		// Perform the Entity-specific simulation.
		universe->Tick(target_frame_seconds);

		// Run physics/collisions.
		physics_engine->Tick(target_frame_seconds);		
		
		// Tell renderer about changes.
		UpdateRenderer();
	}
}

void sim::Simulation::UpdateRenderer() const
{
	gfx::RenderReadyMessage message;
	message.ready = false;
	gfx::Renderer::SendMessage(message);
	
	universe->UpdateModels();
	
	message.ready = true;
	gfx::Renderer::SendMessage(message);
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
			gfx::ResizeMessage message = { Vector2i(event.resize.w, event.resize.h) };
			gfx::Renderer::SendMessage(message);
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
					gfx::Renderer::Ref().ToggleCulling();
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
					gfx::Renderer::Ref().ToggleLighting();
					return true;
					
				case SDLK_o:
					capture = ! capture;
					return true;
					
				case SDLK_p:
					gfx::Renderer::Ref().ToggleWireframe();
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
