//
//  PollCondition.h
//  crag
//
//  Created by John McFarlane on 2012-04-29.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Condition.h"

namespace applet 
{
	// condition type used when calling smp::Daemon::Poll
	class PollCondition : public Condition
	{
	public:
		PollCondition(bool & complete)
		: _complete(complete)
		{
		}
		~PollCondition()
		{
			ASSERT(_complete);
		}
	private:
		virtual bool operator() (bool hurry) final
		{
			return _complete;
		}
		
		// variables
		bool & _complete;
	};
}