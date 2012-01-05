//
//  UserInput.cpp
//  crag
//
//  Created by John on 1/1/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "axes.h"
#include "UserInput.h"

#include "geom/Vector2.h"

#include "core/app.h"


namespace 
{

	template<typename IM> void MapInputs(IM const * mappings, sim::Controller::Impulse & impulse)
	{
		for (IM const * i = mappings; i->affector.type != sim::Controller::Impulse::NUM_TYPES; ++ i) 
		{
			if (i->IsActive()) 
			{
				impulse.factors[i->affector.type][i->affector.axis] += i->affector.delta;
			}
		}
	}


	// Input data

	struct InpulseAffector
	{
		sim::Controller::Impulse::TYPE type;	// [pos/rot]
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
		
		InpulseAffector affector;
		SDL_Scancode key;
	};

	InputKeyMapping const keys[] = 
	{
		// Arrow keys
		{ { sim::Controller::Impulse::TORQUE, axes::UP, +1 }, SDL_SCANCODE_LEFT },
		{ { sim::Controller::Impulse::TORQUE, axes::UP, -1 }, SDL_SCANCODE_RIGHT },
		{ { sim::Controller::Impulse::FORCE, axes::FORWARD, +1 }, SDL_SCANCODE_UP },
		{ { sim::Controller::Impulse::FORCE, axes::FORWARD, -1 }, SDL_SCANCODE_DOWN },
		
		// Above arrow SDL_SCANCODEs (3x2)
		{ { sim::Controller::Impulse::TORQUE, axes::FORWARD, -1 }, SDL_SCANCODE_INSERT },
		{ { sim::Controller::Impulse::FORCE, axes::RIGHT, -1 }, SDL_SCANCODE_DELETE },
		{ { sim::Controller::Impulse::FORCE, axes::FORWARD, +1 }, SDL_SCANCODE_HOME },
		{ { sim::Controller::Impulse::FORCE, axes::FORWARD, -1 }, SDL_SCANCODE_END },
		{ { sim::Controller::Impulse::TORQUE, axes::FORWARD, +1 }, SDL_SCANCODE_PAGEUP },
		{ { sim::Controller::Impulse::FORCE, axes::RIGHT, +1 }, SDL_SCANCODE_PAGEDOWN },
		
		// FPS standard
		{ { sim::Controller::Impulse::TORQUE, axes::FORWARD, -1 }, SDL_SCANCODE_Q },
		{ { sim::Controller::Impulse::FORCE, axes::RIGHT, -1 }, SDL_SCANCODE_A },
		{ { sim::Controller::Impulse::FORCE, axes::FORWARD, +1 }, SDL_SCANCODE_W },
		{ { sim::Controller::Impulse::FORCE, axes::FORWARD, -1 }, SDL_SCANCODE_S },
		{ { sim::Controller::Impulse::TORQUE, axes::FORWARD, +1 }, SDL_SCANCODE_E },
		{ { sim::Controller::Impulse::FORCE, axes::RIGHT, +1 }, SDL_SCANCODE_D },
		
		// Lots of up/down options
		{ { sim::Controller::Impulse::FORCE, axes::UP, +1 }, SDL_SCANCODE_SPACE },
		{ { sim::Controller::Impulse::FORCE, axes::UP, +1 }, SDL_SCANCODE_RSHIFT },
		//{ { sim::Controller::Impulse::FORCE, axes::UP, +1 }, SDL_SCANCODE_LSHIFT },
		{ { sim::Controller::Impulse::FORCE, axes::UP, -1 }, SDL_SCANCODE_RCTRL },
		{ { sim::Controller::Impulse::FORCE, axes::UP, -1 }, SDL_SCANCODE_LCTRL },
		
		{ { sim::Controller::Impulse::FORCE, axes::RIGHT, -1 }, SDL_SCANCODE_COMMA },
		{ { sim::Controller::Impulse::FORCE, axes::RIGHT, +1 }, SDL_SCANCODE_PERIOD },
		
		{ { sim::Controller::Impulse::NUM_TYPES, axes::NUM_AXES, 0 }, SDL_SCANCODE_UNKNOWN }
	};


	// Input mouse data

	struct InputMouseMapping
	{
		bool IsActive() const {
			return app::IsButtonDown(button);
		}
		
		InpulseAffector affector;
		int button;
	};

	InputMouseMapping buttons[] = 
	{
		{ { sim::Controller::Impulse::FORCE, axes::UP, -1 }, 1 },
		{ { sim::Controller::Impulse::FORCE, axes::UP, +1 }, 2 },
		
		{ { sim::Controller::Impulse::NUM_TYPES, axes::NUM_AXES, 0 }, -1 }
	};

}


sim::UserInput::UserInput()
{ 
}

sim::Controller::Impulse sim::UserInput::GetImpulse()
{
	Impulse impulse;
	
	// keyboard
	MapInputs(keys, impulse);
	
	// mouse buttons
	MapInputs(buttons, impulse);

	return impulse;
}
