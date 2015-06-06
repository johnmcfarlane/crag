//
//  MouseObserverController.h
//  crag
//
//  Created by John on 1/24/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Sensor.h"

#include "sim/Controller.h"
#include "sim/defs.h"

#include "core/EventWatcher.h"

namespace physics
{
	class Body;
}

namespace sim
{
	struct ObserverInput;

	// controls an entity whose job it is to translate mouse and keyboard events 
	// into camera impulses which are then appied to a physics body
	class MouseObserverController : public Controller
	{
		typedef Controller _super;
	public:
		// functions
		MouseObserverController(Entity & entity);

		CRAG_ROSTER_OBJECT_DECLARE(MouseObserverController);
	private:
		void Tick();

		void HandleEvents(ObserverInput & input);
		void HandleEvent(ObserverInput & input, SDL_Event const & event);
		void HandleKeyboardEvent(SDL_Scancode scancode, bool down);
		void HandleMouseMove(ObserverInput & input, SDL_MouseMotionEvent const & motion) const;

		//Vector3 MouseDeltaToRotation(MouseDelta const & mouse_delta) const;
		void ScaleInput(ObserverInput & input) const;
		void ApplyInput(ObserverInput const & input);

		void UpdateCamera() const;

		physics::Body & GetBody();
		physics::Body const & GetBody() const;

		// variables
		int _speed;
		core::EventWatcher _event_watcher;
		bool _collidable;
		std::unique_ptr<Sensor> _sensor;
	};
}
