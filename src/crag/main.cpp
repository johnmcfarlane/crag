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

#include "smp/ForEach.h"

#include "physics/Singleton.h"
#include "cl/Singleton.h"


//////////////////////////////////////////////////////////////////////
// Local Function Declarations

namespace 
{
	bool Crag();
	void Sandpit();
}


//////////////////////////////////////////////////////////////////////
// main

int main(int /*argc*/, char * * /*argv*/)
{
#if ! defined(NDEBUG)
	Sandpit();
#endif
	
	std::cout << "Crag Demo\n";
	std::cout << "Copyright 2010 John McFarlane\n";
	
	core::ConfigManager config_manager;
	
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
		if (! sys::Init(Vector2i(video_resolution_x, video_resolution_y), video_full_screen, video_vsync, "Crag"))
		{
			return false;
		}

		smp::Init(0);

		physics::Singleton physics_singleton;
#if defined(USE_OPENCL)
		cl::Singleton cl_singleton;
#endif
		sim::Universe universe;
		
		// Run the simulation.
		{
			sim::Simulation simulation (video_vsync);
			simulation.Run();
		}

		smp::Deinit();

		return true;
	}
	
	void Sandpit()
	{
		int const max_depth = 4;
		int const array_size = 1 << max_depth;
		float array [array_size];
		ZeroArray (array, array_size);
		
		array [(array_size >> 2) * 0] = 0.0;
		array [(array_size >> 2) * 2] = 1.0;
		
		int phase_depth = max_depth - 2;
		for (int depth = max_depth - 1; depth > 0; -- depth)
		{
			int step = 1 << depth;
			int stagger = step >> 1;
			
			for (int i = stagger; i < array_size; i += step)
			{
				int sample_indices [4] = 
				{
					(array_size + i - stagger - step) % array_size,
					(array_size + i - stagger) % array_size,
					(array_size + i + stagger) % array_size,
					(array_size + i + stagger + step) % array_size
				};
				
				float samples[4] = 
				{
					array[sample_indices[0]], 
					array[sample_indices[1]], 
					array[sample_indices[2]], 
					array[sample_indices[3]]
				};
				
				Assert(array[i] == 0);
				if (depth >= phase_depth)
				{
					array[i] = (samples[1] + samples[2]) * .5f;
				}
				else
				{
					array[i] = (samples[0] + samples[3]) * .5f;
				}
			}
		}
		
		for (int i = 0; i < array_size; ++ i)
		{
			std::cout << array[i] << '\n';
		}
	}
}
