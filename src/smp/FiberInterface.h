//
//  FiberInterface.h
//  crag
//
//  Created by John McFarlane on 2012-03-31.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace smp
{
	////////////////////////////////////////////////////////////////////////////////
	// FiberInterface - used to yield/quit from the Fiber that is currently running
	
	class FiberInterface
    {
		OBJECT_NO_COPY(FiberInterface);
    public:
		FiberInterface() = default;
		virtual ~FiberInterface() = default;
		
		// Return execution to the calling thread.
		virtual void Yield() = 0;
		
		// Stop execution on the fiber altogether.
		// It's unwise to call this function directly;
		// it is far better to return out of the fiber.
		virtual void Kill() = 0;
    };
}
