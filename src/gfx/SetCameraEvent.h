//
//  gfx/SetCameraEvent.h
//  crag
//
//  Created by John on 13/05/28.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

#include "geom/Space.h"

namespace gfx
{
	struct SetCameraEvent
	{
		geom::uni::Transformation transformation;
		Scalar fov;
	};
}
