//
//  scripts/ga/AnimatController.h
//  crag
//
//  Created by John McFarlane on 2013-03-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

// TODO: directory structure clearly isn't nailed down yet
#include "scripts/vehicle/sim/VehicleController.h"

#include "Genome.h"
#include "Sensor.h"

namespace sim
{
	// governs the behevior of a sim::Entity which is an artificial animal
	class AnimatController : public VehicleController
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		DECLARE_ALLOCATOR(AnimatController);

		AnimatController(Entity & entity);
		~AnimatController();

		virtual void Tick() final;

	private:
		void CreateSensors();
		void CreateThrusters();
		void Connect();

		void AddSensor(Ray3 const & ray);

		virtual void TickThrusters() final;

		////////////////////////////////////////////////////////////////////////////////
		// variables

		std::vector<Sensor *> _sensors;
		ga::Genome _genome;
	};
}
