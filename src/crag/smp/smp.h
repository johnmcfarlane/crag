/*
 *  smp.h
 *  crag
 *
 *  Created by John on 11/2/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


namespace smp
{
	// Sleep the thread for at least the given amount of time.
	void Sleep(double seconds = 0);
	
	// Return a best estimate at the number of cores/CPUs on the host system
	int GetNumCpus();
}
