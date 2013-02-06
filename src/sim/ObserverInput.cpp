//
//  ObserverInput.cpp
//  crag
//
//  Created by John on 1/1/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "ObserverInput.h"

#include "axes.h"

#include "core/app.h"

using namespace sim;

namespace 
{
	template<typename IM> void MapInputs(IM const * mappings, ObserverInput & input)
	{
		for (IM const * i = mappings; i->affector.type != ObserverInput::size; ++ i) 
		{
			if (i->IsActive()) 
			{
				input[i->affector.type][ i->affector.axis] += i->affector.delta;
			}
		}
	}


	// Input data

	struct InputAffector
	{
		ObserverInput::Index type;	// [pos/rot]
		axes::Axis axis;	// [x/y/z]
		float delta;
	};


	// Input keyboard data

	struct InputKeyMapping 
	{
		bool IsActive() const
		{
			return app::IsKeyDown(key);
		}
		
		InputAffector affector;
		SDL_Scancode key;
	};

	InputKeyMapping const keys[] = 
	{
		// Arrow keys
		{ { ObserverInput::rotation, axes::UP, +1 }, SDL_SCANCODE_LEFT },
		{ { ObserverInput::rotation, axes::UP, -1 }, SDL_SCANCODE_RIGHT },
		{ { ObserverInput::translation, axes::FORWARD, +1 }, SDL_SCANCODE_UP },
		{ { ObserverInput::translation, axes::FORWARD, -1 }, SDL_SCANCODE_DOWN },
		
		// Above arrow SDL_SCANCODEs (3x2)
		{ { ObserverInput::rotation, axes::FORWARD, -1 }, SDL_SCANCODE_INSERT },
		{ { ObserverInput::translation, axes::RIGHT, -1 }, SDL_SCANCODE_DELETE },
		{ { ObserverInput::translation, axes::FORWARD, +1 }, SDL_SCANCODE_HOME },
		{ { ObserverInput::translation, axes::FORWARD, -1 }, SDL_SCANCODE_END },
		{ { ObserverInput::rotation, axes::FORWARD, +1 }, SDL_SCANCODE_PAGEUP },
		{ { ObserverInput::translation, axes::RIGHT, +1 }, SDL_SCANCODE_PAGEDOWN },
		
		// FPS standard
		{ { ObserverInput::rotation, axes::FORWARD, -1 }, SDL_SCANCODE_Q },
		{ { ObserverInput::translation, axes::RIGHT, -1 }, SDL_SCANCODE_A },
		{ { ObserverInput::translation, axes::FORWARD, +1 }, SDL_SCANCODE_W },
		{ { ObserverInput::translation, axes::FORWARD, -1 }, SDL_SCANCODE_S },
		{ { ObserverInput::rotation, axes::FORWARD, +1 }, SDL_SCANCODE_E },
		{ { ObserverInput::translation, axes::RIGHT, +1 }, SDL_SCANCODE_D },
		
		// Lots of up/down options
		{ { ObserverInput::translation, axes::UP, +1 }, SDL_SCANCODE_SPACE },
		{ { ObserverInput::translation, axes::UP, -1 }, SDL_SCANCODE_RCTRL },
		{ { ObserverInput::translation, axes::UP, -1 }, SDL_SCANCODE_LCTRL },

		{ { ObserverInput::size, axes::NUM_AXES, 0 }, SDL_SCANCODE_UNKNOWN }
	};


	// Input mouse data

	struct InputMouseMapping
	{
		bool IsActive() const {
			return app::IsButtonDown(button);
		}
		
		InputAffector affector;
		int button;
	};

	InputMouseMapping buttons[] = 
	{
		{ { ObserverInput::translation, axes::UP, -1 }, 1 },
		{ { ObserverInput::translation, axes::UP, +1 }, 2 },
		
		{ { ObserverInput::size, axes::NUM_AXES, 0 }, -1 }
	};

}

ObserverInput::ObserverInput()
{
	(*this)[0] = value_type::Zero();
	(*this)[1] = value_type::Zero();
}

ObserverInput sim::GetObserverInput()
{
	ObserverInput input;
	
	// keyboard
	MapInputs(keys, input);
	
	// mouse buttons
	MapInputs(buttons, input);

	return input;
}
