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
#include "sim/defs.h"

#include "core/EventWatcher.h"

namespace physics
{
	class Body;
}

namespace sim
{
	class Sensor;

	// controls an entity whose job it is to translate mouse and keyboard events 
	// into camera impulses which are then appied to a physics body
	class CameraController : public Controller
	{
		typedef Controller _super;
	public:
		// functions
		CameraController(Entity & entity, EntityHandle subject);
		virtual ~CameraController();

	private:
		void Tick();

		void HandleEvents();
		void HandleEvent(SDL_Event const & event);
		void HandleKeyboardEvent(SDL_Scancode scancode, bool down);

		void UpdateCamera() const;

		physics::Body & GetBody();
		physics::Body const & GetBody() const;

		// variables
		bool _collidable;
		core::EventWatcher _event_watcher;
		Sensor & _sensor;
		EntityHandle _subject;
	};
}
