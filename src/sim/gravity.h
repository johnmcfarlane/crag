//
//  gravity.h
//  crag
//
//  Created by John McFarlane on 11/6/11.
//  Copyright 2009 - 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

namespace sim
{
	// simulates gravitational interactions between the given entities
	void ApplyGravity(Entity::List & entities, Time delta);
}
