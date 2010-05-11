/*
 *  Controller.h
 *  Crag
 *
 *  Created by John on 1/24/10.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "core/Vector3.h"


namespace sim
{
	class Controller
	{
	public:
		virtual ~Controller();
		
		struct Impulse
		{
			Impulse();

			enum TYPE
			{
				FORCE,
				TORQUE,
				NUM_TYPES
			};

			Vector3f factors[NUM_TYPES];
		};
		
		virtual Impulse GetImpulse() = 0;
	};
}
