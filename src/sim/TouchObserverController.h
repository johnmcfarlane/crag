//
//  TouchObserverController.h
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

	// controls an entity whose job it is to translate touch-screen events into 
	// camera movement
	class TouchObserverController : public Controller
	{
		// types
		struct Finger
		{
			Vector2 down_position;
			Vector2 position;
			SDL_FingerID id;
		};
		
		typedef std::vector<Finger> FingerVector;

	public:
		// functions
		TouchObserverController(Entity & entity, Vector3 const & position);
		virtual ~TouchObserverController();

	private:
		void Tick();

		void HandleEvents();
		void HandleEvent(SDL_Event const & event);
		void HandleFingerDown(Vector2 const & normalized_position, SDL_FingerID id);
		void HandleFingerUp(SDL_FingerID id);
		void HandleFingerMotion(Vector2 const & normalized_position, SDL_FingerID id);

		physics::Body & GetBody();
		physics::Body const & GetBody() const;
		
		FingerVector::iterator FindFinger(SDL_FingerID id);

		static Vector2 NormalizedToScreen(Vector2 normalized_position);
		
		// variables
		Vector3 _position;
		FingerVector _fingers;
		core::EventWatcher _event_watcher;
		bool _collidable;
	};
}
