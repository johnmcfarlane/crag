//
//  smp.cpp
//  crag
//
//  Created by John on 11/2/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "smp.h"

void smp::Yield()
{
	std::this_thread::yield();
}

void smp::Sleep(core::Time seconds)
{
	ASSERT(seconds >= 0);

	auto microseconds = core::SecondsToDuration<std::chrono::microseconds>(seconds);
	std::this_thread::sleep_for(microseconds);
}

size_t smp::GetNumCpus()
{
	return std::thread::hardware_concurrency();
}
