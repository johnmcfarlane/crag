//
//  main.h
//  crag
//
//  Created by John McFarlane on 2012-02-09.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "Fiber.h"

#include "sim/Simulation.h"


namespace sim
{
	class Planet;
	class Star;
	class Vehicle;
}

namespace script
{
	// the entry point for all scripts
	class MainFunctor
	{
	public:
		// types
		typedef std::vector<smp::Handle <sim::Vehicle> > EntityVector;
		
		// functions
		void operator() (FiberInterface & fiber);
	private:
		void SpawnUniverse();
		void SpawnVehicle();
		void SpawnShapes();

		// variables
		smp::Handle<sim::Planet> _planet, _moon1, _moon2;
		smp::Handle<sim::Star> _sun;
		smp::Handle<sim::Vehicle> _vehicle;
		EntityVector _shapes;
	};
}
