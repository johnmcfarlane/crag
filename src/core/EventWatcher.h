//
//  EventWatcher.h
//  crag
//
//  Created by John McFarlane on 2/17/12.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "smp/SimpleMutex.h"


namespace core
{
	// Condition that an event has not arrived in the applet::Engine.
	class EventWatcher
	{
	public:
		// functions
		EventWatcher();
		~EventWatcher();

		bool IsEmpty();
		bool PopEvent(SDL_Event & event);
	private:
		// SDL callback
		static int OnEvent(void *userdata, SDL_Event * event);
		
		// variables
		std::vector<SDL_Event> _events;
		smp::SimpleMutex _mutex;
	};
}
