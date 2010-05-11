/*
 *  Controller.cpp
 *  Crag
 *
 *  Created by John on 1/24/10.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#include "pch.h"

#include "Controller.h"


sim::Controller::~Controller()
{
}

sim::Controller::Impulse::Impulse()
{
	factors[FORCE] = Vector3f::Zero();
	factors[TORQUE] = Vector3f::Zero();
}
