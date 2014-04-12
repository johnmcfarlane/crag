//
//  entity/sim/UfoController.h
//  crag
//
//  Created by John McFarlane on 2014-03-20.
//	Copyright 2014 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "entity/sim/VehicleController.h"

#include "gfx/SetCameraEvent.h"

#include "ipc/Listener.h"

#include "core/EventWatcher.h"

namespace sim
{
	// controls a vessel with mouse or touch-based tilting behavior
	class UfoController final
	: public VehicleController
	, private ipc::Listener<Engine, gfx::SetCameraEvent>
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		UfoController(Entity & entity);
		~UfoController();

		CRAG_VERIFY_INVARIANTS_DECLARE(UfoController);

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
		
		Thruster * _main_thruster;
		int _num_presses;
	};
}
