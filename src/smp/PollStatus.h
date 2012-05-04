//
//  PollStatus.h
//  crag
//
//  Created by John McFarlane on 2012-05-03.
//  Copyright 2009-2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace smp
{
	// stores the result from cross-thread poll commands, 
	// usually in futures
	enum PollStatus
	{
		pending,	// result has not yet been written
		complete,	// result has been written
		failed		// target object did not exist
	};
}
