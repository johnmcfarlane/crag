/*
 *  UserInput.h
 *  Crag
 *
 *  Created by John on 1/1/10.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

// TODO: This is all very poorly defined.

#pragma once

#include "Controller.h"


////////////


namespace sim
{
	class UserInput : public Controller
	{
	public:
		UserInput();

		Impulse GetImpulse();
	};
}
