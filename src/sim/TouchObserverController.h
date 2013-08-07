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

#include "gfx/Pov.h"

#include "ipc/Listener.h"

#include "core/EventWatcher.h"

#include "geom/origin.h"
#include "geom/Transformation.h"

namespace physics
{
	class Body;
}

namespace gfx
{
	struct SetOriginEvent;
}

namespace sim
{
	struct ObserverInput;

	// controls an entity whose job it is to translate touch-screen events into 
	// camera movement
	class TouchObserverController 
	: public Controller
	, private ipc::Listener<Engine, gfx::SetOriginEvent>
	{
		// types
		struct Finger;
		typedef std::vector<Finger> FingerVector;

	public:
		// functions
		TouchObserverController(Entity & entity);
		virtual ~TouchObserverController();

	private:
		void Tick();

		void operator() (gfx::SetOriginEvent const & event) final;

		void HandleEvents();
		void HandleEvent(SDL_Event const & event);
		void HandleFingerDown(Vector2 const & screen_position, SDL_FingerID id);
		void HandleFingerUp(SDL_FingerID id);
		void HandleFingerMotion(Vector2 const & screen_position, SDL_FingerID id);
		
		void UpdateCamera();
		void UpdateCamera(Finger const & finger);
		void UpdateCamera(Finger const & finger1, Finger const & finger2);
		
		Transformation const & GetTransformation() const;
		void SetTransformation(Transformation const & transformation);
		void BroadcastTransformation() const;

		physics::Body & GetBody();
		physics::Body const & GetBody() const;
		
		FingerVector::iterator FindFinger(SDL_FingerID id);
		FingerVector::const_iterator FindFinger(SDL_FingerID id) const;
		bool IsDown(SDL_FingerID id) const;
		
		Vector2 GetScreenPosition(SDL_TouchFingerEvent const & touch_finger_event) const;
		Vector2 GetScreenPosition(SDL_MouseButtonEvent const & mouse_button_event) const;
		Vector2 GetScreenPosition(SDL_MouseMotionEvent const & mouse_motion_event) const;

		Vector3 GetPixelDirection(Vector2 const & screen_position, Transformation const & transformation) const;
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		// used to calculate screen->world projection
		gfx::Frustum _frustum;
		
		// local origin in universal space
		geom::abs::Vector3 _origin;
		
		// camera transformation at point where finger pressed/released
		Transformation _down_transformation;
		
		// record of fingers used to interact with touch screen
		FingerVector _fingers;
		
		// receives/stores touch events
		core::EventWatcher _event_watcher;
	};
}
