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

// TODO: Remove SDLmain files from OS X project?
//#include <SDL_main.h>

#include <fstream>


//////////////////////////////////////////////////////////////////////
// Local Function Declarations

namespace ANONYMOUS 
{
	bool Crag();
	bool Load();
	bool Save();
}


//////////////////////////////////////////////////////////////////////
// main

int main(int /*argc*/, char * * /*argv*/)
{
	std::cout << "Crag Demo\n";
	std::cout << "Copyright 2010 John McFarlane\n";
	
	if (! Load())
	{
		// Make sure we always have a config file handy.
		Save();
	}
	
	bool ok = Crag();
	
	Save();
	
	return ok
	? EXIT_SUCCESS
	: EXIT_FAILURE;
}

//////////////////////////////////////////////////////////////////////
// Local Variables


#if defined(ENABLE_CONFIG)
#define LOAD_CONFIG 1
#define SAVE_CONFIG 1
#else
#define LOAD_CONFIG 0
#define SAVE_CONFIG 0
#endif


namespace ANONYMOUS {

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

	// Run the simulation.
	sim::Simulation * sim = new sim::Simulation ();
	sim->Run();
	delete sim;

	return true;
}

bool Load()
{
#if (LOAD_CONFIG == 1)
	std::ifstream in(config_filename);

	if (in.is_open())
	{
		ConfigManager::Load(in);
		return true;
	}
	else
	{
		std::cout << "Failed to open config file \"" << config_filename << "\" for reading.\n";
		return false;
	}
#elif (LOAD_CONFIG == 0)
	return false;
#endif
}

bool Save()
{
#if (SAVE_CONFIG == 1)
	std::ofstream out(config_filename);

	if (out.is_open())
	{
		ConfigManager::Save(out);
		return true;
	}
	else
	{
		std::cout << "Failed to open config file \"" << config_filename << "\" for writing.\n";
		return false;
	}
#elif (SAVE_CONFIG == 0)
	return false;
#endif
}

}
