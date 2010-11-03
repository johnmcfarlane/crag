/*
 *  smp.cpp
 *  crag
 *
 *  Created by John on 11/2/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "smp.h"

#if defined(WIN32)
#include <WinBase.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#else
#endif


void smp::Sleep(double seconds)
{
	SDL_Delay(Uint32(seconds * 1000));
}


int smp::GetNumCpus()
{
#if defined(WIN32)
	SYSTEM_INFO sysinfo;
	GetSystemInfo (& sysinfo);
	return sysinfo.dwNumberOfProcessors;
#elif defined(__APPLE__)
	int num_cpus;
    size_t len = sizeof(num_cpus);
    int mib[2] = { CTL_HW, HW_NCPU };
    if (sysctl(mib, 2, & num_cpus, &len, NULL, 0) == 0)
	{
		return num_cpus;
	}
	
	Assert(false);
	return 1;
#else 
    // Linux, Solaris, Tru64, UnixWare 7, and Open UNIX 8
	// Assumes defined(_SC_NPROCESSORS_ONLN).
    int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	if (num_cpus != -1)
	{
		return num_cpus;
	}
	
	Assert(false);
	return 1;
#endif
}
