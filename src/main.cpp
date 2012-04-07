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

#include "form/Engine.h"
#include "sim/Engine.h"
#include "gfx/Engine.h"

#include "applet/Engine.h"
#include "applet/Applet.h"
#include "applet/TestScript.h"


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

	printf("Crag Demo\n"
		   "Copyright 2010-2012 John McFarlane\n");
	
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
						sim::Daemon::Call(& sim::Engine::OnTogglePause);
						return true;
					}
					
					case SDL_SCANCODE_B:
					{
						gfx::Daemon::Call(& gfx::Engine::OnToggleCulling);
						return true;
					}
					
					case SDL_SCANCODE_F:
						form::Daemon::Call(& form::Engine::OnToggleFlatShaded);
						return true;
						
					case SDL_SCANCODE_G:
					{
						sim::Daemon::Call(& sim::Engine::OnToggleGravity);
						return true;
					}
						
					case SDL_SCANCODE_I:
						form::Daemon::Call(& form::Engine::OnToggleSuspended);
						return true;
						
					case SDL_SCANCODE_L:
					{
						gfx::Daemon::Call(& gfx::Engine::OnToggleLighting);
						return true;
					}
					
					case SDL_SCANCODE_P:
					{
						gfx::Daemon::Call(& gfx::Engine::OnToggleWireframe);
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
						sim::Daemon::Call(& sim::Engine::OnToggleCollision);
						return true;
					}
					
					case SDL_SCANCODE_I:
						form::Daemon::Call(& form::Engine::OnToggleMeshGeneration);
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
						form::Daemon::Call(& form::Engine::OnToggleDynamicOrigin);
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
						gfx::Daemon::Call(& gfx::Engine::OnToggleCapture);
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
		if (! app::GetEvent(event))
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
						gfx::Daemon::Call(& gfx::Engine::OnResize, size);
						return true;
					}
					
					case SDL_WINDOWEVENT_SHOWN:
					case SDL_WINDOWEVENT_EXPOSED:
					case SDL_WINDOWEVENT_MAXIMIZED:
					case SDL_WINDOWEVENT_RESTORED:
					case SDL_WINDOWEVENT_ENTER:
					case SDL_WINDOWEVENT_FOCUS_GAINED:
					{
						form::Daemon::Call(& form::Engine::OnRegulatorSetEnabled, true);
						return true;
					}
					
					case SDL_WINDOWEVENT_HIDDEN:
					case SDL_WINDOWEVENT_MINIMIZED:
					case SDL_WINDOWEVENT_LEAVE:
					case SDL_WINDOWEVENT_FOCUS_LOST:
					{
						form::Daemon::Call(& form::Engine::OnRegulatorSetEnabled, false);
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

		// If not caught here, then send it to the application event queue.
		app::PushEvent(event);
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
			// Instantiate the four daemons
			gfx::Daemon renderer(0x8000);
			form::Daemon formation(0x8000);
			sim::Daemon simulation(0x400);
			script::Daemon scripting(0x400);
			
			// start thread the daemons
			formation.Start("form");
			simulation.Start("sim");
			renderer.Start("render");
			scripting.Start("script");
			
			// launch the main script
			script::Daemon::Call(& script::Engine::Launch, & script::Test);
			
			while (true)
			{
				if (! formation.IsRunning())
				{
					DEBUG_MESSAGE("formation initiating shutdown");
					break;
				}
				if (! simulation.IsRunning())
				{
					DEBUG_MESSAGE("simulation initiating shutdown");
					break;
				}
				if (! renderer.IsRunning())
				{
					DEBUG_MESSAGE("renderer initiating shutdown");
					break;
				}
				if (! scripting.IsRunning())
				{
					DEBUG_MESSAGE("scripting initiating shutdown");
					break;
				}

				if (! HandleEvent())
				{
					// TODO: This call is costly and shouldn't be necessary with a few fixes to shut-down code. 
					smp::Yield();
				}
			}
			
			// Tell the daemons to wind down.
			scripting.BeginFlush();
			simulation.BeginFlush();
			renderer.BeginFlush();
			formation.BeginFlush();
			
			// Wait until they have all stopped working.
			scripting.Synchronize();
			simulation.Synchronize();
			renderer.Synchronize();
			formation.Synchronize();
			
			// Wait until they have all finished flushing.
			scripting.EndFlush();
			simulation.EndFlush();
			renderer.EndFlush();
			formation.EndFlush();
		}
		
		smp::scheduler::Deinit();
		app::Deinit();
		
		return true;
	}
}
