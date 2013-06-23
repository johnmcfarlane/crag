//
//  gfx/SetOriginEvent.h
//  crag
//
//  Created by John on 2013-06-17.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "geom/origin.h"

namespace gfx
{
	// inform listeners that the relative origin has changed
	struct SetOriginEvent
	{
		typedef geom::abs::Vector3 Vector;
		
		// the global position of the relative origin
		Vector origin;
	};
}
