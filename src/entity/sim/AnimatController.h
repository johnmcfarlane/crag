//
//  scripts/ga/AnimatController.h
//  crag
//
//  Created by John McFarlane on 2013-03-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "entity/sim/VehicleController.h"

#include "Genome.h"

namespace sim
{
	// forward-declarations
	class Sensor;
	
	// governs the behevior of a sim::Entity which is an artificial animal
	class AnimatController : public VehicleController
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		AnimatController(Entity & entity, float radius);
		~AnimatController();

		static void Tick(AnimatController *);

	private:
		void CreateSensors(float radius);
		void CreateThrusters(float radius);
		void Connect();

		void AddSensor(Ray3 const & ray);

		virtual void TickThrusters() final;

		////////////////////////////////////////////////////////////////////////////////
		// variables

		std::vector<std::unique_ptr<Sensor>> _sensors;
		ga::Genome _genome;
	};
}
