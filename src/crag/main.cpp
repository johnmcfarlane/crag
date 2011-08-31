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
	CONFIG_DEFINE (video_full_screen, bool, true);
#endif


	//////////////////////////////////////////////////////////////////////
	// Local Function Definitions


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
			script::ScriptThread::Daemon script_daemon(0x400);
			gfx::Renderer::Daemon renderer(0x8000);
			form::FormationManager::Daemon formation_manager(0x8000);
			sim::Simulation::Daemon simulation(0x400);
			
			// start thread for three of the daemons
			formation_manager.Start();
			simulation.Start();
			renderer.Start();
			
			// Launch the script engine daemon.
			// It needs to run in the main thread because SDL was initialized from here
			// and script uses SDL events fns. Hence we call Run instead of Start etc..
			script_daemon.Run();
			// The script daemon returning is the signal for the program to finish.
			
			// Tell the daemons to wind down.
			script_daemon.RequestStop();
			simulation.RequestStop();
			renderer.RequestStop();
			formation_manager.RequestStop();

			// Wait until they have all stopped working.
			script_daemon.AcknowledgeStop();
			simulation.AcknowledgeStop();
			renderer.AcknowledgeStop();
			formation_manager.AcknowledgeStop();
		}
		
		smp::scheduler::Deinit();
		sys::Deinit();

		return true;
	}
}
