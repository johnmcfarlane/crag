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

#include "sim/Simulation.h"
#include "sim/Universe.h"

#include "core/ConfigEntry.h"
#include "core/ConfigManager.h"
#include "core/Statistics.h"

#include "smp/ForEach.h"

#include "physics/Singleton.h"

#include "vm/Singleton.h"


//////////////////////////////////////////////////////////////////////
// Local Function Declarations

namespace 
{
	bool Crag();
}


//////////////////////////////////////////////////////////////////////
// main

int main(int /*argc*/, char * * /*argv*/)
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
		
		smp::Init(0);

		physics::Singleton physics_singleton;
		sim::Universe universe;
		vm::Singleton virtual_machine;
		
		// Run the simulation.
		{
			sim::Simulation simulation (video_vsync);
			simulation.Run();
		}

		smp::Deinit();

		return true;
	}
}
