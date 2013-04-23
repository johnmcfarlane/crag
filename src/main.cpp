//
// main.cpp
// Crag
//
// Created by john on 6/13/09.
// Copyright 2009, 2010 John McFarlane. All rights reserved.
// This program is distributed under the terms of the GNU General Public License.
//


#include "pch.h"

#include "scripts/MainScript.h"

#include "form/Engine.h"
#include "sim/Engine.h"
#include "gfx/Engine.h"

#include "applet/Engine.h"
#include "applet/Applet.h"

#include "smp/scheduler.h"

#include "core/app.h"
#include "core/ConfigManager.h"

#define RUN_TEST


//////////////////////////////////////////////////////////////////////
// Local Function Declarations

namespace 
{
	bool CragMain();
}


//////////////////////////////////////////////////////////////////////
// main

#if defined(WIN32)
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	CragMain();

	// WM_QUIT param should be returned here but it is lost
	return 0;
}
#else
int main(int, char * *)
{
	if (CragMain())
	{
		return EXIT_SUCCESS;
	}
	else
	{
		return EXIT_FAILURE;
	}
}
#endif	// ! WIN32

//////////////////////////////////////////////////////////////////////
// Local Variables


CONFIG_DEFINE (profile_mode, bool, false);


namespace 
{

#if ! defined(WIN32)
	CONFIG_DEFINE (video_resolution_x, int, 800);
	CONFIG_DEFINE (video_resolution_y, int, 600);
#else
	CONFIG_DEFINE (video_resolution_x, int, 800);
	CONFIG_DEFINE (video_resolution_y, int, 600);
#endif
	
#if defined(PROFILE)
	CONFIG_DEFINE (video_full_screen, bool, false);
#elif defined(NDEBUG)
	CONFIG_DEFINE (video_full_screen, bool, true);
#else
	CONFIG_DEFINE (video_full_screen, bool, false);
#endif
	
	
	//////////////////////////////////////////////////////////////////////
	// Local Function Definitions
	
	// returns false iff the program should quit.
	void OnKeyPress(SDL_Keysym keysym)
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
						sim::Daemon::Call([] (sim::Engine & engine) { engine.OnTogglePause(); });
						break;
					}
					
					case SDL_SCANCODE_B:
					{
						gfx::Daemon::Call([] (gfx::Engine & engine) { engine.OnToggleCulling(); });
						break;
					}
					
					case SDL_SCANCODE_F:
						gfx::Daemon::Call([] (gfx::Engine & engine) { engine.OnToggleFlatShaded(); });
						break;

					case SDL_SCANCODE_G:
					{
						sim::Daemon::Call([] (sim::Engine & engine) { engine.OnToggleGravity(); });
						break;
					}
						
					case SDL_SCANCODE_I:
						form::Daemon::Call([] (form::Engine & engine) { engine.OnToggleSuspended(); });
						break;
						
					case SDL_SCANCODE_L:
					{
						gfx::Daemon::Call([] (gfx::Engine & engine) { engine.OnToggleLighting(); });
						break;
					}
					
					case SDL_SCANCODE_P:
					{
						gfx::Daemon::Call([] (gfx::Engine & engine) { engine.OnToggleWireframe(); });
						break;
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
						sim::Daemon::Call([] (sim::Engine & engine) { engine.OnToggleCollision(); });
						break;
					
					case SDL_SCANCODE_I:
						form::Daemon::Call([] (form::Engine & engine) { engine.OnToggleMeshGeneration(); });
						break;
						
					case SDL_SCANCODE_F:
						gfx::Daemon::Call([] (gfx::Engine & engine) { engine.OnToggleFragmentLighting(); });
						break;
						
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
						gfx::Daemon::Call([] (gfx::Engine & engine) { engine.OnToggleCapture(); });
						break;
					}
					
					default:
						break;
				}
				break;
			}
			
			default:
				break;
		}
	}
	
	// Returns false if it's time to quit.
	bool HandleEvent()
	{
		SDL_Event event;
		
		if (! app::GetEvent(event))
		{
			return true;
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
						geom::Vector2i size(window_event.data1, window_event.data2);
						gfx::Daemon::Call([size] (gfx::Engine & engine) {
							engine.OnResize(size);
						});
						break;
					}
					
					case SDL_WINDOWEVENT_SHOWN:
					case SDL_WINDOWEVENT_EXPOSED:
					case SDL_WINDOWEVENT_MAXIMIZED:
					case SDL_WINDOWEVENT_RESTORED:
					case SDL_WINDOWEVENT_ENTER:
					case SDL_WINDOWEVENT_FOCUS_GAINED:
					{
						form::Daemon::Call([] (form::Engine & engine) {
							engine.EnableAdjustNumQuaterna(true);
						});
						break;
					}
					
					case SDL_WINDOWEVENT_HIDDEN:
					case SDL_WINDOWEVENT_MINIMIZED:
					case SDL_WINDOWEVENT_LEAVE:
					case SDL_WINDOWEVENT_FOCUS_LOST:
					{
						form::Daemon::Call([] (form::Engine & engine) {
							engine.EnableAdjustNumQuaterna(false);
						});
						break;
					}
				}
				break;
			}
				
			case SDL_KEYDOWN:
			{
				OnKeyPress(event.key.keysym);
				break;
			}
			
			case SDL_QUIT:
			{
				// it's time to quit
				return false;
			}
		}

		return true;
	}
	
	int EventFilter(void *, SDL_Event * event)
	{
		switch (event->type) 
		{
			case SDL_MOUSEMOTION:
			case SDL_QUIT:
			case SDL_WINDOWEVENT:
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				return 1;
				
			default:
				return 0;
		}
	}

	// The main program function.
	bool CragMain()
	{
		printf("Crag Demo\n"
			   "Copyright 2010-2012 John McFarlane\n");

		DEBUG_MESSAGE("-> CragMain");

		// Instance the config manager first of all so that all the config variables, such as video_full_screen are correct.
		core::ConfigManager config_manager;
		
		geom::Vector2i video_resolution(video_resolution_x, video_resolution_y);
		bool full_screen = (! profile_mode) && video_full_screen;
		if (! app::Init(video_resolution, full_screen, "Crag"))
		{
			return false;
		}
		
		SDL_SetEventFilter(EventFilter, nullptr);
		
		// sheduler and formation thread are always running
		size_t num_reserved_cpus = 2;
		smp::scheduler::Init(num_reserved_cpus);
		
		{
			// TODO: Find a way to make these common; writing everything out four times is not good.
			// Instantiate the four daemons
			gfx::Daemon renderer(0x8000);
			form::Daemon formation(0x8000);
			sim::Daemon simulation(0x800);
			applet::Daemon applets(0x400);
			
			// start thread the daemons
			formation.Start("form");
			simulation.Start("sim");
			renderer.Start("render");
			applets.Start("applet");
			
			// launch the main script
			applet::AppletHandle::CreateHandle("Main", 8192, & MainScript);
			
			while (HandleEvent())
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
				if (! applets.IsRunning())
				{
					DEBUG_MESSAGE("applets initiating shutdown");
					break;
				}
			}
			
			// Tell the daemons to wind down.
			applets.BeginFlush();
			simulation.BeginFlush();
			renderer.BeginFlush();
			formation.BeginFlush();
			
			// Wait until they have all stopped working.
			applets.Synchronize();
			simulation.Synchronize();
			renderer.Synchronize();
			formation.Synchronize();
			
			// Wait until they have all finished flushing.
			applets.EndFlush();
			simulation.EndFlush();
			renderer.EndFlush();
			formation.EndFlush();
		}
		
		smp::scheduler::Deinit();
		app::Deinit();
		
		DEBUG_MESSAGE("<- CragMain");
		return true;
	}
}
