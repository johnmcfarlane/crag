//
//  gfx/SetLodParametersEvent.h
//  crag
//
//  Created by John on 2014/03/30.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "LodParameters.h"

namespace gfx
{
	struct SetLodParametersEvent
	{
		LodParameters parameters;
	};
}
