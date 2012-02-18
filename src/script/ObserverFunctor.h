//
//  ObserverFunctor.h
//  crag
//
//  Created by John McFarlane on 2012-02-11.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Fiber.h"

#include "EntityHandle.h"
#include "EventCondition.h"

#include "sim/Observer.h"


namespace script
{
	class ObserverFunctor
	{
	public:
		// functions
		ObserverFunctor(sim::Vector3 const & spawn_position);
		~ObserverFunctor();
		
		void operator() (FiberInterface & fiber);
		
	private:
		bool HandleEvent(SDL_Event const & event);
		bool HandleKeyboardEvent(SDL_Scancode scancode, bool down);
		bool HandleMouseButton(Uint8 button, bool down);
		bool HandleMouseMove(int x_delta, int y_delta);
		
		void SetSpeed(int speed);
		
		// variables
		EntityHandle<sim::Observer> _observer;
		EventCondition _event_condition;
		bool _collidable;
	};
}
