//
// Crag.cpp
// Crag
//
// Created by john on 6/13/09.
// Copyright 2009, 2010 John McFarlane. All rights reserved.
// This program is distributed under the terms of the GNU General Public License.
//


#include "pch.h"

#include "core/ConfigManager.h"

#include "smp/scheduler.h"

#include "form/FormationManager.h"
#include "sim/Simulation.h"
#include "gfx/Renderer.h"

#include "script/ScriptThread.h"

#include <fstream>


//////////////////////////////////////////////////////////////////////
// Local Function Declarations

namespace 
{
	bool Crag(char const * program_path);
}


//////////////////////////////////////////////////////////////////////
// main

int SDL_main(int /*argc*/, char * * argv)
{
#if defined(WIN32)
	std::ofstream cout_filestr, cerr_filestr;
	cout_filestr.open ("cout.txt");
	cerr_filestr.open ("cerr.txt");
	std::cout.rdbuf(cout_filestr.rdbuf());
	std::cerr.rdbuf(cerr_filestr.rdbuf());
#endif

	std::cout << "Crag Demo" << std::endl;
	std::cout << "Copyright 2010 John McFarlane" << std::endl;
	
	return Crag(* argv) ? EXIT_SUCCESS : EXIT_FAILURE;
}

//////////////////////////////////////////////////////////////////////
// Local Variables


namespace 
{

	CONFIG_DEFINE (video_resolution_x, int, 800);
	CONFIG_DEFINE (video_resolution_y, int, 600);
	
#if defined(PROFILE)
	CONFIG_DEFINE (video_full_screen, bool, false);
#else
	CONFIG_DEFINE (video_full_screen, bool, false);
#endif
	
	
	//////////////////////////////////////////////////////////////////////
	// Local Function Definitions
	
	// returns false iff the program should quit.
	bool OnKeyPress(sys::KeyCode key_code)
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
					{
						sim::TogglePauseMessage message;
						sim::Simulation::Daemon::SendMessage(message);
						return true;
					}
						
					case SDLK_c:
						gfx::Renderer::Daemon::Ref().ToggleCulling();
						return true;
						
					case SDLK_f:
						form::FormationManager::Daemon::Ref().ToggleFlatShaded();
						return true;
						
					case SDLK_g:
					{
						sim::ToggleGravityMessage message;
						sim::Simulation::Daemon::SendMessage(message);
						return true;
					}
						
					case SDLK_i:
						form::FormationManager::Daemon::Ref().ToggleSuspended();
						return true;
						
					case SDLK_l:
						gfx::Renderer::Daemon::Ref().ToggleLighting();
						return true;
						
					case SDLK_o:
					{
						gfx::ToggleCaptureMessage message;
						gfx::Renderer::Daemon::SendMessage(message);
						return true;
					}
						
					case SDLK_p:
						gfx::Renderer::Daemon::Ref().ToggleWireframe();
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
						sim::ToggleCollisionMessage message;
						sim::Simulation::Daemon::SendMessage(message);
						return true;
					}
						
					case SDLK_i:
						form::FormationManager::Daemon::Ref().ToggleMeshGeneration();
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
						form::FormationManager::Daemon::Ref().ToggleDynamicOrigin();
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
	
	bool HandleEvent()
	{
		script::EventMessage event_message;
		
		// If no events are pending,
		if (! sys::GetEvent(event_message.event, false))
		{
			// then nothing's happening event-wise.
			return false;
		}
		
		switch (event_message.event.type)
		{
			case SDL_VIDEORESIZE:
			{
				gfx::ResizeMessage message = { Vector2i(event_message.event.resize.w, event_message.event.resize.h) };
				gfx::Renderer::Daemon::SendMessage(message);
				return true;
			}
				
			case SDL_KEYDOWN:
			{
				if (OnKeyPress(event_message.event.key.keysym.sym))
				{
					return true;
				}
				break;
			}
				
			case SDL_ACTIVEEVENT:
			{
				form::RegulatorResetMessage message;
				form::FormationManager::Daemon::SendMessage(message);			
				return true;
			}
				
			default:
			{
				break;
			}
		}

		// If not caught here, then send it to the script thread.
		script::ScriptThread::Daemon::SendMessage(event_message);
		return true;
	}
	
	bool Crag(char const * program_path)
	{
		// Instance the config manager first of all so that all the config variables, such as video_full_screen are correct.
		core::ConfigManager config_manager;
		
		if (! sys::Init(Vector2i(video_resolution_x, video_resolution_y), video_full_screen, "Crag", program_path))
		{
			return false;
		}
		
#if defined (GATHER_STATS)
		core::Statistics stat_manager;
#endif
		
		smp::scheduler::Init();
		
		{
			// Instanciate the four daemons
			gfx::Renderer::Daemon renderer(0x8000);
			form::FormationManager::Daemon formation_manager(0x8000);
			sim::Simulation::Daemon simulation(0x400);
			script::ScriptThread::Daemon script_daemon(0x400);
			
			// start thread the daemons
			formation_manager.Start();
			simulation.Start();
			renderer.Start();
			script_daemon.Start();
			
			while (script_daemon.IsRunning())
			{
				if (! HandleEvent())
				{
					smp::Yield();
				}
			}
			
			// Tell the daemons to wind down.
			script_daemon.BeginFlush();
			simulation.BeginFlush();
			renderer.BeginFlush();
			formation_manager.BeginFlush();
			
			// Wait until they have all stopped working.
			script_daemon.Synchronize();
			simulation.Synchronize();
			renderer.Synchronize();
			formation_manager.Synchronize();
			
			// Wait until they have all finished flushing.
			script_daemon.EndFlush();
			simulation.EndFlush();
			renderer.EndFlush();
			formation_manager.EndFlush();
		}
		
		smp::scheduler::Deinit();
		sys::Deinit();

		return true;
	}
}
