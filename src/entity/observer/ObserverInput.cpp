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
				
				int delta = (direction < Direction::negative) ? 1 : -1;
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
		{ { ObserverInput::rotation, Direction::down }, SDL_SCANCODE_LEFT },
		{ { ObserverInput::rotation, Direction::up }, SDL_SCANCODE_RIGHT },
		{ { ObserverInput::translation, Direction::forward }, SDL_SCANCODE_UP },
		{ { ObserverInput::translation, Direction::backward }, SDL_SCANCODE_DOWN },
		
		// Above arrow SDL_SCANCODEs (3x2)
		{ { ObserverInput::rotation, Direction::backward }, SDL_SCANCODE_INSERT },
		{ { ObserverInput::translation, Direction::left }, SDL_SCANCODE_DELETE },
		{ { ObserverInput::translation, Direction::forward }, SDL_SCANCODE_HOME },
		{ { ObserverInput::translation, Direction::backward }, SDL_SCANCODE_END },
		{ { ObserverInput::rotation, Direction::forward }, SDL_SCANCODE_PAGEUP },
		{ { ObserverInput::translation, Direction::right }, SDL_SCANCODE_PAGEDOWN },
		
		// FPS standard
		{ { ObserverInput::rotation, Direction::forward }, SDL_SCANCODE_Q },
		{ { ObserverInput::translation, Direction::left }, SDL_SCANCODE_A },
		{ { ObserverInput::translation, Direction::forward }, SDL_SCANCODE_W },
		{ { ObserverInput::translation, Direction::backward }, SDL_SCANCODE_S },
		{ { ObserverInput::rotation, Direction::backward }, SDL_SCANCODE_E },
		{ { ObserverInput::translation, Direction::right }, SDL_SCANCODE_D },
		
		// Lots of up/down options
		{ { ObserverInput::translation, Direction::up }, SDL_SCANCODE_SPACE },
		{ { ObserverInput::translation, Direction::down }, SDL_SCANCODE_RCTRL },
		{ { ObserverInput::translation, Direction::down }, SDL_SCANCODE_LCTRL },

		{ { ObserverInput::size, Direction::size }, SDL_SCANCODE_UNKNOWN }
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
		{ { ObserverInput::translation, Direction::down }, 1 },
		{ { ObserverInput::translation, Direction::up }, 2 },
		
		{ { ObserverInput::size, Direction::size }, -1 }
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
