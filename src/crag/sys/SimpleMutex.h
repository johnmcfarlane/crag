/*
 *  SimpleMutex.h
 *  crag
 *
 *  Created by John on 8/26/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


namespace sys
{
	
	// Uses Dekker's algorithm to gate access to a resource between two threads: a reader and a writer.
	// This is a naive implementation which may not work on all architectures. 
	// Only use this for brief locks. 
	class SimpleMutex
	{
	public:
		SimpleMutex();
		~SimpleMutex();
		
		void Lock(int i);
		void Unlock(int i);
		
	private:
		
		volatile bool flag[2];
		volatile int turn;
	};

}
