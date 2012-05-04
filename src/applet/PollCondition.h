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
		PollCondition(smp::PollStatus const & status)
		: _status(status)
		{
		}
		~PollCondition()
		{
			ASSERT(_status == smp::complete || _status == smp::failed);
		}
	private:
		virtual bool operator() (bool hurry) final
		{
			return _status != smp::pending;
		}
		
		// variables
		smp::PollStatus const & _status;
	};
}