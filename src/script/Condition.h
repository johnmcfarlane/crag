//
//  Condition.h
//  crag
//
//  Created by John McFarlane on 2/17/12.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace script
{
	// forward-declarations
	class Engine;
	
	// Somewhat like a thread condition applied, instead, to a fiber.
	// The fiber is paused until its associated condition tests true.
	class Condition
	{
	public:
		virtual ~Condition() { }
		virtual bool operator() (Engine & engine) = 0;
	};
}
