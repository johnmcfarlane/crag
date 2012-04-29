//
//  Condition.h
//  crag
//
//  Created by John McFarlane on 2/17/12.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace applet
{
	// forward-declarations
	class Engine;
	
	// Somewhat like a thread condition applied, instead, to a fiber.
	// The fiber is paused until its associated condition tests true.
	class Condition
	{
	public:
		virtual ~Condition() { }
		
		// true iff the condition is satisfied and the fiber can continue
		virtual bool operator() (bool hurry) = 0;
	};
}
