//
//  gfx/SetSpaceEvent.h
//  crag
//
//  Created by John on 2013-06-17.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "geom/Space.h"

namespace gfx
{
	// inform listeners that the relative space has changed;
	// all relative coordinates must pass through geom::Convert
	struct SetSpaceEvent
	{
		geom::Space space;
	};
}
