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

#include "physics/RayCast.h"

namespace physics
{
	class Body;
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
		CRAG_ROSTER_OBJECT_DECLARE(CameraController);

		CameraController(Entity & entity, std::shared_ptr<Entity> const & subject);

	private:
		void Tick();
		void UpdateCameraRayCast() const;

		physics::Body & GetBody();
		physics::Body const & GetBody() const;

		// variables
		std::unique_ptr<physics::RayCast> _ray_cast;
		std::shared_ptr<Entity> _subject;
	};
}
