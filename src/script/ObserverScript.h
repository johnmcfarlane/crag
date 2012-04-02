//
//  ObserverScript.h
//  crag
//
//  Created by John McFarlane on 2012-02-11.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "EventCondition.h"

#include "Script.h"

#include "sim/Simulation.h"


DECLARE_CLASS_HANDLE(sim, Observer)	// sim::ObserverHandle


namespace script
{
	class ObserverScript : public Script
	{
	public:
		// functions
		ObserverScript(sim::Vector3 const & spawn_position);
		~ObserverScript();
		
		virtual void operator() (FiberInterface & fiber) override;
		
	private:
		bool HandleEvent(SDL_Event const & event);
		bool HandleKeyboardEvent(SDL_Scancode scancode, bool down);
		bool HandleMouseButton(Uint8 button, bool down);
		bool HandleMouseMove(int x_delta, int y_delta);
		
		void SetSpeed(int speed);
		
		// variables
		sim::ObserverHandle _observer;
		EventCondition _event_condition;
		bool _collidable;
	};
}
