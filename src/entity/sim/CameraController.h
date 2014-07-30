//
//  CameraController.h
//  crag
//
//  Created by John on 2014-01-30.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "sim/Controller.h"

namespace physics
{
	class Body;
	class RayCast;
}

namespace sim
{
	class Sensor;

	// controller for a camera entity that tracks a subject
	// and dictates global camera position
	class CameraController : public Controller
	{
		// types
		typedef Controller _super;
		
	public:
		// functions
		CameraController(Entity & entity, std::shared_ptr<Entity> const & subject);
		virtual ~CameraController();

	private:
		void Tick();
		void UpdateCameraRayCast() const;

		physics::Body & GetBody();
		physics::Body const & GetBody() const;

		// variables
		physics::RayCast & _ray_cast;
		std::shared_ptr<Entity> _subject;
	};
}
