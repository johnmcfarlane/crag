//
//  entity/sim/UfoController1.h
//  crag
//
//  Created by John McFarlane on 2014-03-20.
//	Copyright 2014 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "VehicleController.h"

#include <sim/Engine.h>

#include "gfx/SetCameraEvent.h"

#include "core/EventWatcher.h"

namespace sim
{
	// controls a vessel with mouse or touch-based tilting behavior
	class UfoController1 final
	: public VehicleController
	, private ipc::Listener<Engine, gfx::SetCameraEvent>
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		CRAG_ROSTER_OBJECT_DECLARE(UfoController1);

		UfoController1(Entity & entity, std::shared_ptr<Entity> const & ball_entity, Scalar max_thrust);
		~UfoController1();

		CRAG_VERIFY_INVARIANTS_DECLARE(UfoController1);

	private:
		void Tick();

		void ApplyThrust(Vector2 pointer_delta);
		bool ShouldThrust(bool is_rotating) const;

		void ApplyTilt(Vector2 pointer_delta);

		Vector2 HandleEvents();
		Vector2 HandleEvent(SDL_Event const & event);
		void HandleKeyboardEvent(SDL_Scancode scancode, bool down);

		void operator() (gfx::SetCameraEvent const & event);

		////////////////////////////////////////////////////////////////////////////////
		// data

		core::EventWatcher _event_watcher;
		
		Matrix33 _camera_rotation;
		
		std::shared_ptr<Entity> _ball_entity;
		int _num_presses;
	};
}
