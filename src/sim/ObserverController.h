//
//  ObserverController.h
//  crag
//
//  Created by John on 1/24/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

#include "Controller.h"

#include "core/EventWatcher.h"

namespace physics
{
	class Body;
}

namespace sim
{
	struct ObserverInput;

	class ObserverController : public Controller
	{
		typedef Controller _super;
	public:
		// functions
		ObserverController(Entity & entity);
		virtual ~ObserverController();

	private:		
		virtual void Tick() final;

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
	};
}
