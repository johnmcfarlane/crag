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


namespace script
{
	// Condition that an event has not arrived in the script::Engine.
	class EventCondition : public Condition
	{
	public:
		// functions
		bool operator() (Engine & engine);
		SDL_Event const & GetEvent() const;
		
	private:
		// variables
		SDL_Event _event;
	};
}
