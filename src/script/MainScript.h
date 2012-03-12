//
//  MainScript.h
//  crag
//
//  Created by John McFarlane on 2012-02-09.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "Fiber.h"

#include "Script.h"

#include "sim/Simulation.h"


DECLARE_CLASS_HANDLE(script, MainScript);	// script::MainScriptHandle

DECLARE_CLASS_HANDLE(sim, Planet);	// sim::PlanetHandle
DECLARE_CLASS_HANDLE(sim, Star);	// sim::StarHandle
DECLARE_CLASS_HANDLE(sim, Vehicle);	// sim::VehicleHandle


namespace script
{
	// the entry point for all scripts
	class MainScript : public Script
	{
	public:
		// types
		typedef std::vector<sim::VehicleHandle> EntityVector;
		
		// functions
		virtual void operator() (FiberInterface & fiber) override;
	private:
		void SpawnUniverse();
		void SpawnVehicle();
		void SpawnShapes();

		// variables
		sim::PlanetHandle _planet, _moon1, _moon2;
		sim::StarHandle _sun;
		sim::VehicleHandle _vehicle;
		EntityVector _shapes;
	};
}
