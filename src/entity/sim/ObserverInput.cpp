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

#include "gfx/axes.h"

#include "core/app.h"

using namespace sim;
using gfx::Direction;

namespace 
{
	template<typename IM> void MapInputs(IM const * mappings, ObserverInput & input)
	{
		for (IM const * i = mappings; i->affector.type != ObserverInput::size; ++ i) 
		{
			if (i->IsActive()) 
			{
				auto & affector = i->affector;
				auto direction = affector.direction;
				
				int delta = affector.pole_sign;
				auto axis_index = static_cast<int>(direction) % 3;
				input[affector.type][axis_index] += delta;
			}
		}
	}


	// Input data

	struct InputAffector
	{
		ObserverInput::Index type;	// [pos/rot]
		Direction direction;
		int pole_sign;
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
		{ { ObserverInput::rotation, Direction::up, -1 }, SDL_SCANCODE_LEFT },
		{ { ObserverInput::rotation, Direction::up, 1 }, SDL_SCANCODE_RIGHT },
		{ { ObserverInput::translation, Direction::forward, 1 }, SDL_SCANCODE_UP },
		{ { ObserverInput::translation, Direction::forward, -1 }, SDL_SCANCODE_DOWN },
		
		// Above arrow SDL_SCANCODEs (3x2)
		{ { ObserverInput::rotation, Direction::forward, -1 }, SDL_SCANCODE_INSERT },
		{ { ObserverInput::translation, Direction::right, -1 }, SDL_SCANCODE_DELETE },
		{ { ObserverInput::translation, Direction::forward, 1 }, SDL_SCANCODE_HOME },
		{ { ObserverInput::translation, Direction::forward, -1 }, SDL_SCANCODE_END },
		{ { ObserverInput::rotation, Direction::forward, 1 }, SDL_SCANCODE_PAGEUP },
		{ { ObserverInput::translation, Direction::right, 1 }, SDL_SCANCODE_PAGEDOWN },
		
		// FPS standard
		{ { ObserverInput::rotation, Direction::forward, 1 }, SDL_SCANCODE_Q },
		{ { ObserverInput::translation, Direction::right, -1 }, SDL_SCANCODE_A },
		{ { ObserverInput::translation, Direction::forward, 1 }, SDL_SCANCODE_W },
		{ { ObserverInput::translation, Direction::forward, -1 }, SDL_SCANCODE_S },
		{ { ObserverInput::rotation, Direction::forward, -1 }, SDL_SCANCODE_E },
		{ { ObserverInput::translation, Direction::right, 1 }, SDL_SCANCODE_D },
		
		// Lots of up/down options
		{ { ObserverInput::translation, Direction::up, 1 }, SDL_SCANCODE_SPACE },
		{ { ObserverInput::translation, Direction::up, -1 }, SDL_SCANCODE_RCTRL },
		{ { ObserverInput::translation, Direction::up, -1 }, SDL_SCANCODE_LCTRL },

		{ { ObserverInput::size, Direction::size, 0 }, SDL_SCANCODE_UNKNOWN }
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
		{ { ObserverInput::size, Direction::size, 0 }, -1 }
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
