//
//  debug.cpp
//  crag
//
//  Created by John on 2/10/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "core/debug.h"

#if ! defined(NDEBUG) && ! defined(WIN32)

ReentryGuard::ReentryGuard(int & counter)
: _counter(counter) 
{ 
	assert(++ _counter == 1); 

} 

ReentryGuard::~ReentryGuard() 
{ 
	assert(-- _counter == 0); 
}

#endif

