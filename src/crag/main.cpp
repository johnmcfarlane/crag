
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

#include <SDL/SDL.h>

#include <fstream>


#define LOAD_CONFIG 0


//////////////////////////////////////////////////////////////////////
// Local Function Declarations

namespace ANONYMOUS {

bool Crag();
bool Load();
bool Save();

}


//////////////////////////////////////////////////////////////////////
// main

int main(int /*argc*/, char * * /*argv*/)
{
	std::cout << "Crag Demo (c)2010 John McFarlane\n";
	
	if (! Load()) {
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


namespace ANONYMOUS {

CONFIG_DEFINE (resolution_x, int, 1000);
CONFIG_DEFINE (resolution_y, int, 750);

#if defined(FULL_SCREEN)
#if defined(NDEBUG)
CONFIG_DEFINE (fullscreen, bool, false);
#else
CONFIG_DEFINE (fullscreen, bool, false);
#endif
#endif

char const config_filename[] = "crag.txt";

//const std::string title = "Crag";


//////////////////////////////////////////////////////////////////////
// Local Function Definitions


bool Crag()
{
#if FULL_SCREEN
	bool full_screen = true;
#else
	bool full_screen = false;
#endif

	if (! app::Init(Vector2i(resolution_x, resolution_y), full_screen))
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
		std::cerr << "Failed to open config file \"" << config_filename << "\" for reading.\n";
		return false;
	}
#elif (LOAD_CONFIG == 0)
	return false;
#endif
}

bool Save()
{
	std::ofstream out(config_filename);

	if (out.is_open())
	{
		ConfigManager::Save(out);
		return true;
	}
	else
	{
		std::cerr << "Failed to open config file \"" << config_filename << "\" for writing.\n";
		return false;
	}
}

}


#if 0

/*
 * Required libs/APIs/Framworks/Fireworks/Dreamworks:
 * 		GL/GLU
 * 		SDL
 * 		ODE double-precision
 * 		boost
 *
 * Instructions:
 *      General:
 *		debug defines: PROFILE
 *		retail defines: NDEBUG
 *		ODE needs to be double-precision
 *
 *	Windows:
 *		TODO!
 *
 *      Linux (i.e. Ubuntu):
 *      	Install GL/GLU (packages with mesa and/or dev)
 *      	Install SDL development packages
 *      	Install ODE
 *      	Install boost
 *
 *		XCode: Out of date!
 *			install SFML (including XCode project templates)
 *			Create SFML Graphics Project (as opposed to a windows project); I call mine 'Crag' :)
 *			download/extract ODE
 *			move "ode-0.11.1/build/config-default.h" to "ode-0.11.1/ode/src/config.h"
 *			In the project view, ctrl-click the 'Sources' folder and "Add->Existing Files" and point to "ode-0.11.1/ode/src"; name it 'ode' to avoid confusion.
 *			In the project view, ctrl-click the 'Sources' folder and "Add->Existing Files" and point to "ode-0.11.1/OPCODE"; name it 'ode' to avoid confusion.
 *			remove all the makefile.* files and the config.h.in from the various ODE folders
 *          Add "ode-0.11.1/include" to "Header Search Path" setting
 *          Add "ode-0.11.1/ode/src" to "User Header Search Path" setting
 *			Also add "src" to "User Header Search Path" with "Recursive" on
 *			might also add include path "ode-0.11.1/ode/src"
 *			Add OpenGL Framework to the frameworks folder of the project
 *
 *		Eclipse:
 *			Copy the src folder into the workspace and refresh the project
 *			In the project properties:
 *				in C/C++ General -> Paths and Symbols
 *					in Includes -> GNU C++
 *						add workspace folder, "src/crag"
 *						add filesystem folder, "/System/Library/Frameworks/OpenGL.framework/Headers"
 *					in Symbols -> GNU C++ 
 *						Add symbol "PROFILE" to the Debug configuration
 *						Add symbol "NDEBUG" to the Release configuration
 *				in C/C++ Build -> Settings
 *					in Tool Settings -> MacOS X C++ Linker
 *						in -> Libraries
 *							Add Libraries: SDL, ode, boost_thread, SDL_image and SDLmain
 *						in -> Miscellaneous
 *							Add Linker flags: "-framework OpenGL -framework Cocoa"
 *
 */

TODO {
	ode		formation collision;
	feat	Tunnels/Craters;
	misc	Clicker/tally: class that counts the c'tors / d'tors and asserts at program end;
	coord	Add in other planets again;
	form	fix seams;
	form	Fix the fractal;
	gfx		cull faces based on normal?

	gfx		Tidy up Push/PopAttribs (www.mesa3d.org/brianp/sig97/perfopt.htm);
	gfx		adaptive z-near/far
	opt		Z-sort
	opt		Use intrinsics in dRecipSqrt and other ODE fns
	opt		Could we upload the verts prior to the indices? They get generated much sooner!
	opt		Keep track of min and max 'dirty' verts (and maybe also indices) and only upload those.


	gfx		Fix Lighting
	opt		Clip whole planets - will cut them completely out of shadow maps in many cases
	feat	normalize observer impulse
	form	Imposing a max angle on the fractal will allow many optimizations
	form	Consider removing the corners pointers as they are generated along the way
	form	Different vertices for corners at each level of tesselation?
	gfx		Advanced Skybox
	gfx		GLEW MX - need this? 
}
#endif
