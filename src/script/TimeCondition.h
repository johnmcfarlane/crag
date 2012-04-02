//
//  TimeCondition.h
//  crag
//
//  Created by John McFarlane on 2012-02-17.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Condition.h"


namespace script
{
	// Condition that passes on, or after, a position in time.
	class TimeCondition : public Condition
	{
		// functions
	public:
		TimeCondition(Time wake_position);
	private:
		bool operator() (script::Engine & script_thread);
		
		// variables
		Time _wake_position;
	};
}
