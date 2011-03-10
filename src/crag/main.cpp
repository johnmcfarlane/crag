/*
 *  Crag.cpp
 *  Crag
 *
 *  Created by john on 6/13/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */


#include "pch.h"

#include "core/ConfigManager.h"

#include "smp/ForEach.h"
#include "smp/Thread.h"

#include "physics/Singleton.h"
#include "form/FormationManager.h"
#include "sim/Simulation.h"


#include "script/ScriptThread.h"


//////////////////////////////////////////////////////////////////////
// Local Function Declarations

namespace 
{
	bool Crag();
}


//////////////////////////////////////////////////////////////////////
// main

int SDL_main(int /*argc*/, char * * /*argv*/)
{
	std::cout << "Crag Demo\n";
	std::cout << "Copyright 2010 John McFarlane\n";
	
	return Crag() ? EXIT_SUCCESS : EXIT_FAILURE;
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
	
	CONFIG_DEFINE (video_vsync, bool, true);
	
	char const config_filename[] = "crag.cfg";


	//////////////////////////////////////////////////////////////////////
	// Local Function Definitions


	bool Crag()
	{
		// Instance the config manager first of all so that all the config variables, such as video_full_screen are correct.
		core::ConfigManager config_manager;
		
		if (! sys::Init(Vector2i(video_resolution_x, video_resolution_y), video_full_screen, video_vsync, "Crag"))
		{
			return false;
		}
		
#if defined (GATHER_STATS)
		core::Statistics stat_manager;
#endif
		
		smp::Init(1);

		physics::Singleton physics;
		
		{
			form::FormationManager formation_manager;
			smp::Thread<form::FormationManager> formation_thread;
			formation_thread.Launch<& form::FormationManager::Run>(formation_manager);
			
			sim::Simulation simulation(true);
			smp::Thread<sim::Simulation> simulation_thread;
			simulation_thread.Launch<& sim::Simulation::Run>(simulation);
			
			// Launch the script engine.
			// Note: this needs to run in the main thread because SDL
			// was initialized from here and script uses SDL events fns. 
			script::ScriptThread script_thread;
			script_thread.Run("./script/main.py");

			{
				sim::Simulation::ptr sim_lock = sim::Simulation::GetLock();
				form::FormationManager::ptr form_lock = form::FormationManager::GetLock();
				simulation.Exit();
				formation_manager.Exit();
			}
		}
		
		smp::Deinit();
		sys::Deinit();

		return true;
	}
}
