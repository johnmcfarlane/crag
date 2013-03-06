//
//  scripts/ga/AnimatController.h
//  crag
//
//  Created by John McFarlane on 2013-03-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "scripts/vehicle/sim/VehicleController.h"

namespace sim
{
	// governs the behevior of a sim::Entity which is an artificial animal
	class AnimatController : public VehicleController
	{
		class Sensor
		{
		public:
			Sensor(Ray3 const & position);
		private:
			Ray3 _ray;	// Project(_ray, 1) = average sensor tip
			std::vector<float> _thruster_mapping;
		};

	public:
		AnimatController(Entity & entity);

	private:
		void CreateSensors();
		void CreateThrusters();
		void Connect();

		void AddSensor(Ray3 const & ray);

		virtual void Tick() final;

		virtual void TickThrusters() final;

		std::vector<Sensor> _sensors;
	};
}
