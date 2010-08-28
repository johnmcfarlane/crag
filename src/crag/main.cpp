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

#include "core/ConfigEntry.h"
#include "core/ConfigManager.h"

#include "sys/Scheduler.h"

#include "physics/Singleton.h"
#include "cl/Singleton.h"
#include "gfx/Renderer.h"


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
	
	if (! ConfigManager::Load())
	{
		// Make sure we always have a config file handy.
		ConfigManager::Save();
	}
	
	bool ok = Crag();
	
	ConfigManager::Save();
	
	return ok
	? EXIT_SUCCESS
	: EXIT_FAILURE;
}

//////////////////////////////////////////////////////////////////////
// Local Variables


namespace 
{

	CONFIG_DEFINE (resolution_x, int, 960);
	CONFIG_DEFINE (resolution_y, int, 720);

	#if defined(PROFILE)
		CONFIG_DEFINE (full_screen, bool, false);
	#else
		CONFIG_DEFINE (full_screen, bool, true);
	#endif

	char const config_filename[] = "crag.cfg";


	//////////////////////////////////////////////////////////////////////
	// Local Function Definitions


	bool Crag()
	{
		if (! app::Init(Vector2i(resolution_x, resolution_y), full_screen, "Crag"))
		{
			return false;
		}
		
		physics::Singleton physics_singleton;
#if defined(USE_OPENCL)
		cl::Singleton cl_singleton;
#endif
		gfx::Renderer renderer;
		sys::Scheduler scheduler(1);
		
		// Run the simulation.
		{
			sim::Simulation simulation;
			simulation.Run();
		}

		return true;
	}
}
