//
// main.cpp
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
	std::cout << "Copyright 2010 John McFarlane" << std::endl;
	
	return CragMain(* argv) ? EXIT_SUCCESS : EXIT_FAILURE;
}

//////////////////////////////////////////////////////////////////////
// Local Variables


CONFIG_DEFINE (profile_mode, bool, false);


namespace 
{

	CONFIG_DEFINE (video_resolution_x, int, 800);
	CONFIG_DEFINE (video_resolution_y, int, 600);
	
#if defined(PROFILE)
	CONFIG_DEFINE (video_full_screen, bool, false);
#else
	CONFIG_DEFINE (video_full_screen, bool, true);
#endif
	
	
	//////////////////////////////////////////////////////////////////////
	// Local Function Definitions
	
	// returns false iff the program should quit.
	bool OnKeyPress(SDL_Keysym keysym)
	{
		switch (keysym.mod) 
		{
			case KMOD_NONE:
			{
				switch (keysym.scancode)
				{
					case SDL_SCANCODE_RETURN:
					{
						sim::TogglePauseMessage message;
						sim::Simulation::Daemon::SendMessage(message);
						return true;
					}
					
					case SDL_SCANCODE_B:
					{
						gfx::ToggleCullingMessage message;
						gfx::Renderer::Daemon::SendMessage(message);
						return true;
					}
					
					case SDL_SCANCODE_F:
						form::FormationManager::Daemon::Ref().ToggleFlatShaded();
						return true;
						
					case SDL_SCANCODE_G:
					{
						sim::ToggleGravityMessage message;
						sim::Simulation::Daemon::SendMessage(message);
						return true;
					}
						
					case SDL_SCANCODE_I:
						form::FormationManager::Daemon::Ref().ToggleSuspended();
						return true;
						
					case SDL_SCANCODE_L:
					{
						gfx::ToggleLightingMessage message;
						gfx::Renderer::Daemon::SendMessage(message);
						return true;
					}
						
					case SDL_SCANCODE_O:
					{
						gfx::ToggleCaptureMessage message;
						gfx::Renderer::Daemon::SendMessage(message);
						return true;
					}
						
					case SDL_SCANCODE_P:
					{
						gfx::ToggleWireframeMessage message;
						gfx::Renderer::Daemon::SendMessage(message);
						return true;
					}
						
					default:
						break;
				}
				break;
			}
				
			case KMOD_LSHIFT:
			case KMOD_RSHIFT:
			{
				switch (keysym.scancode)
				{
					case SDL_SCANCODE_C:
					{
						sim::ToggleCollisionMessage message;
						sim::Simulation::Daemon::SendMessage(message);
						return true;
					}
					
					case SDL_SCANCODE_I:
						form::FormationManager::Daemon::Ref().ToggleMeshGeneration();
						return true;
						
					default:
						break;
				}
				break;
			}
				
			case KMOD_LCTRL:
			case KMOD_RCTRL:
			{
				switch (keysym.scancode)
				{
					case SDL_SCANCODE_I:
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
			case SDL_WINDOWEVENT:
			{
				SDL_WindowEvent const & window_event = event_message.event.window;
				switch (window_event.event)
				{
					case SDL_WINDOWEVENT_RESIZED:
					{
						// TODO: Check it's the right window?
						gfx::ResizeMessage message = { Vector2i(window_event.data1, window_event.data2) };
						gfx::Renderer::Daemon::SendMessage(message);
						return true;
					}
					
					default:
					{
						// Most window events represent some sort of disruption to the window
						// so reset the regulator.
						form::RegulatorResetMessage message;
						form::FormationManager::Daemon::SendMessage(message);			
						return true;
					}
				}
				break;
			}
				
			case SDL_KEYDOWN:
			{
				if (OnKeyPress(event_message.event.key.keysym))
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
		script::ScriptThread::Daemon::SendMessage(event_message);
		return true;
	}

	// The main program function.
	bool CragMain(char const * program_path)
	{
		// Instance the config manager first of all so that all the config variables, such as video_full_screen are correct.
		core::ConfigManager config_manager;
		
		Vector2i video_resolution(video_resolution_x, video_resolution_y);
		bool full_screen = (! profile_mode) && video_full_screen;
		if (! sys::Init(video_resolution, full_screen, "Crag", program_path))
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
			gfx::Renderer::Daemon renderer(0x8000);
			form::FormationManager::Daemon formation_manager(0x8000);
			sim::Simulation::Daemon simulation(0x400);
			script::ScriptThread::Daemon script_daemon(0x400);
			
			// start thread the daemons
			formation_manager.Start();
			simulation.Start();
			renderer.Start();
			script_daemon.Start();
			
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
