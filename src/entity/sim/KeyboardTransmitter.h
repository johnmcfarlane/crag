//
//  KeyboardTransmitter.h
//  crag
//
//  Created by John McFarlane on 2015-06-11.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Signal.h"

#include <core/RosterObjectDeclare.h>

namespace sim
{
	// transmit one of two values depending on whether key is down
	class KeyboardTransmitter : public Transmitter
	{
	public:
		// functions
		CRAG_ROSTER_OBJECT_DECLARE(KeyboardTransmitter);

		KeyboardTransmitter(SDL_Scancode key, SignalType down_signal = 1.f, SignalType up_signal = 0.f);

		void Tick();

	private:
		// variables
		SDL_Scancode _key;
		SignalType _down_signal;
		SignalType _up_signal;
	};
}
