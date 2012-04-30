//
//  Controller.cpp
//  crag
//
//  Created by John on 1/24/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Controller.h"


sim::Controller::~Controller()
{
}

sim::Controller::Impulse::Impulse()
{
	factors[FORCE] = geom::Vector3f::Zero();
	factors[TORQUE] = geom::Vector3f::Zero();
}
