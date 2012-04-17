//
//  EventCondition.h
//  crag
//
//  Created by John McFarlane on 2/17/12.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Condition.h"

#include "smp/SimpleMutex.h"


namespace applet
{
	// Condition that an event has not arrived in the applet::Engine.
	class EventCondition : public Condition
	{
	public:
		// functions
		EventCondition();
		~EventCondition();

		bool PopEvent(SDL_Event & event);
	private:
		// Condition override
		virtual bool operator() (bool hurry) override;
		
		// SDL callback
		static int OnEvent(void *userdata, SDL_Event * event);
		
		// variables
		std::vector<SDL_Event> _events;
		smp::SimpleMutex _mutex;
	};
}
