//
//  Fiber.h
//  crag
//
//  Created by John McFarlane on 2012-03-28.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#if defined(WIN32)
#include "FiberWin.h"
#else
#include "FiberPosix.h"
#endif
