//
// main.cpp
// Crag
//
// Created by john on 6/13/09.
// Copyright 2009, 2010 John McFarlane. All rights reserved.
// This program is distributed under the terms of the GNU General Public License.
//


#include "pch.h"

#include "core/app.h"
#include "core/ConfigManager.h"

#include "smp/scheduler.h"

#include "form/FormationManager.h"
#include "sim/Simulation.h"
#include "gfx/Renderer.h"

#include "script/ScriptThread.h"


//////////////////////////////////////////////////////////////////////
// Local Function Declarations

namespace 
{
	bool CragMain(char const * program_path);
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
	std::cout << "Copyright 2010-2012 John McFarlane" << std::endl;
	
	int exit_value = CragMain(* argv) ? EXIT_SUCCESS : EXIT_FAILURE;
	
	return exit_value;
}

//////////////////////////////////////////////////////////////////////
// Local Variables


CONFIG_DEFINE (profile_mode, bool, false);


namespace 
{

#if ! defined(WIN32)
	CONFIG_DEFINE (video_resolution_x, int, 800);
	CONFIG_DEFINE (video_resolution_y, int, 600);
#else
	CONFIG_DEFINE (video_resolution_x, int, 1536);
	CONFIG_DEFINE (video_resolution_y, int, 864);
#endif
	
#if defined(PROFILE)
	CONFIG_DEFINE (video_full_screen, bool, false);
#elif defined(WIN32)
	CONFIG_DEFINE (video_full_screen, bool, true);
#else
	CONFIG_DEFINE (video_full_screen, bool, false);
#endif
	
	
	//////////////////////////////////////////////////////////////////////
	// Local Function Definitions
	
	// returns false iff the program should quit.
	bool OnKeyPress(SDL_Keysym keysym)
	{
		// group the mod keys
		Uint16 keys_mods = KMOD_NONE;
		if (keysym.mod & KMOD_SHIFT)
		{
			keys_mods |= KMOD_SHIFT;
		}
		if (keysym.mod & KMOD_CTRL)
		{
			keys_mods |= KMOD_CTRL;
		}
		if (keysym.mod & KMOD_ALT)
		{
			keys_mods |= KMOD_ALT;
		}

		// interpret scancode based on grouped modifiers
		switch (keys_mods)
		{
			case KMOD_NONE:
			{
				switch (keysym.scancode)
				{
					case SDL_SCANCODE_RETURN:
					{
						sim::Daemon::Call(& sim::Simulation::OnTogglePause);
						return true;
					}
					
					case SDL_SCANCODE_B:
					{
						gfx::Daemon::Call(& gfx::Renderer::OnToggleCulling);
						return true;
					}
					
					case SDL_SCANCODE_F:
						form::Daemon::Call(& form::FormationManager::OnToggleFlatShaded);
						return true;
						
					case SDL_SCANCODE_G:
					{
						sim::Daemon::Call(& sim::Simulation::OnToggleGravity);
						return true;
					}
						
					case SDL_SCANCODE_I:
						form::Daemon::Call(& form::FormationManager::OnToggleSuspended);
						return true;
						
					case SDL_SCANCODE_L:
					{
						gfx::Daemon::Call(& gfx::Renderer::OnToggleLighting);
						return true;
					}
					
					case SDL_SCANCODE_P:
					{
						gfx::Daemon::Call(& gfx::Renderer::OnToggleWireframe);
						return true;
					}
					
					default:
						break;
				}
				break;
			}
				
			case KMOD_SHIFT:
			{
				switch (keysym.scancode)
				{
					case SDL_SCANCODE_C:
					{
						sim::Daemon::Call(& sim::Simulation::OnToggleCollision);
						return true;
					}
					
					case SDL_SCANCODE_I:
						form::Daemon::Call(& form::FormationManager::OnToggleMeshGeneration);
						return true;
						
					default:
						break;
				}
				break;
			}
				
			case KMOD_CTRL:
			{
				switch (keysym.scancode)
				{
					case SDL_SCANCODE_I:
						form::Daemon::Call(& form::FormationManager::OnToggleDynamicOrigin);
						return true;
					
					default:
						break;
				}
				break;
			}
				
			case KMOD_CTRL | KMOD_SHIFT:
			{
				switch (keysym.scancode)
				{
					case SDL_SCANCODE_O:
					{
						gfx::Daemon::Call(& gfx::Renderer::OnToggleCapture);
						return true;
					}
					
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
		SDL_Event event;
		
		// If no events are pending,
		if (! app::GetEvent(event, false))
		{
			// then nothing's happening event-wise.
			return false;
		}
		
		switch (event.type)
		{
			case SDL_WINDOWEVENT:
			{
				SDL_WindowEvent const & window_event = event.window;
				switch (window_event.event)
				{
					case SDL_WINDOWEVENT_RESIZED:
					{
						// TODO: Check it's the right window?
						Vector2i size(window_event.data1, window_event.data2);
						gfx::Daemon::Call(& gfx::Renderer::OnResize, size);
						return true;
					}
					
					case SDL_WINDOWEVENT_SHOWN:
					case SDL_WINDOWEVENT_EXPOSED:
					case SDL_WINDOWEVENT_MAXIMIZED:
					case SDL_WINDOWEVENT_RESTORED:
					case SDL_WINDOWEVENT_ENTER:
					case SDL_WINDOWEVENT_FOCUS_GAINED:
					{
						form::Daemon::Call(& form::FormationManager::OnRegulatorSetEnabled, true);
						return true;
					}
					
					case SDL_WINDOWEVENT_HIDDEN:
					case SDL_WINDOWEVENT_MINIMIZED:
					case SDL_WINDOWEVENT_LEAVE:
					case SDL_WINDOWEVENT_FOCUS_LOST:
					{
						form::Daemon::Call(& form::FormationManager::OnRegulatorSetEnabled, false);
						return true;
					}
				}
				break;
			}
				
			case SDL_KEYDOWN:
			{
				if (OnKeyPress(event.key.keysym))
				{
					return true;
				}
				break;
			}
				
			default:
			{
				break;
			}
		}

		// If not caught here, then send it to the script thread.
		script::Daemon::Call(& script::ScriptThread::OnEvent, event);
		return true;
	}

	// The main program function.
	bool CragMain(char const * program_path)
	{
		// Instance the config manager first of all so that all the config variables, such as video_full_screen are correct.
		core::ConfigManager config_manager;
		
		Vector2i video_resolution(video_resolution_x, video_resolution_y);
		bool full_screen = (! profile_mode) && video_full_screen;
		if (! app::Init(video_resolution, full_screen, "Crag", program_path))
		{
			return false;
		}
		
#if defined (GATHER_STATS)
		core::Statistics stat_manager;
#endif
		
		smp::scheduler::Init();
		
		{
			// TODO: Find a way to make these common; writing everything out four times is not good.
			// Instanciate the four daemons
			gfx::Daemon renderer(0x8000);
			form::Daemon formation_manager(0x8000);
			sim::Daemon simulation(0x400);
			script::ScriptThread::Daemon script_daemon(0x400);
			
			// start thread the daemons
			formation_manager.Start("form");
			simulation.Start("sim");
			renderer.Start("gfx");
			script_daemon.Start("script");
			
			while (true)
			{
				if (! formation_manager.IsRunning())
				{
					std::cout << "formation_manager initiating shutdown" << std::endl;
					break;
				}
				if (! simulation.IsRunning())
				{
					std::cout << "simulation initiating shutdown" << std::endl;
					break;
				}
				if (! renderer.IsRunning())
				{
					std::cout << "renderer initiating shutdown" << std::endl;
					break;
				}
				if (! script_daemon.IsRunning())
				{
					std::cout << "script_daemon initiating shutdown" << std::endl;
					break;
				}

				if (! HandleEvent())
				{
					// TODO: This call is costly and shouldn't be necessary with a few fixes to shut-down code. 
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
		app::Deinit();
		
		return true;
	}
}
