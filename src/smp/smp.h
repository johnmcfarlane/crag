//
//  smp.h
//  crag
//
//  Created by John on 11/2/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


// general purpose functions related to Symmetric MultiProcessing
namespace smp
{
	// Move on to another thread.
	void Yield();
	
	// Sleep the thread for at least the given amount of time.
	void Sleep(core::Time duration);
	
	// Return a best estimate at the number of cores/CPUs on the host system
	unsigned GetNumCpus();
}
