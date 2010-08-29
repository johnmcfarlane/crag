/*
 *  UserInput.cpp
 *  Crag
 *
 *  Created by John on 1/1/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "axes.h"
#include "UserInput.h"

#include "geom/Vector2.h"

#include "sys/App.h"


namespace 
{

	template<typename IM> void MapInputs(IM const * mappings, sim::Controller::Impulse & impulse)
	{
		for (IM const * i = mappings; i->affector.type != sim::Controller::Impulse::NUM_TYPES; ++ i) {
			if (i->IsActive()) {
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
			return sys::IsKeyDown(key);
		}
		
		InpulseAffector affector;
		sys::KeyCode key;
	};

	InputKeyMapping const keys[] = 
	{
		// Arrow keys
		{ { sim::Controller::Impulse::FORCE, axes::RIGHT, -1 }, KEY_LEFT },
		{ { sim::Controller::Impulse::FORCE, axes::RIGHT, +1 }, KEY_RIGHT },
		{ { sim::Controller::Impulse::FORCE, axes::FORWARD, +1 }, KEY_UP },
		{ { sim::Controller::Impulse::FORCE, axes::FORWARD, -1 }, KEY_DOWN },
		
		// Above arrow keys (3x2)
		{ { sim::Controller::Impulse::TORQUE, axes::FORWARD, -1 }, KEY_INSERT },
		{ { sim::Controller::Impulse::FORCE, axes::RIGHT, -1 }, KEY_DELETE },
		{ { sim::Controller::Impulse::FORCE, axes::FORWARD, +1 }, KEY_HOME },
		{ { sim::Controller::Impulse::FORCE, axes::FORWARD, -1 }, KEY_END },
		{ { sim::Controller::Impulse::TORQUE, axes::FORWARD, +1 }, KEY_PAGEUP },
		{ { sim::Controller::Impulse::FORCE, axes::RIGHT, +1 }, KEY_PAGEDOWN },
		
		// FPS standard
		{ { sim::Controller::Impulse::TORQUE, axes::FORWARD, -1 }, KEY_Q },
		{ { sim::Controller::Impulse::FORCE, axes::RIGHT, -1 }, KEY_A },
		{ { sim::Controller::Impulse::FORCE, axes::FORWARD, +1 }, KEY_W },
		{ { sim::Controller::Impulse::FORCE, axes::FORWARD, -1 }, KEY_S },
		{ { sim::Controller::Impulse::TORQUE, axes::FORWARD, +1 }, KEY_E },
		{ { sim::Controller::Impulse::FORCE, axes::RIGHT, +1 }, KEY_D },
		
		// Lots of up/down options
		{ { sim::Controller::Impulse::FORCE, axes::UP, +1 }, KEY_SPACE },
		{ { sim::Controller::Impulse::FORCE, axes::UP, +1 }, KEY_RSHIFT },
		//{ { sim::Controller::Impulse::FORCE, axes::UP, +1 }, KEY_LSHIFT },
		{ { sim::Controller::Impulse::FORCE, axes::UP, -1 }, KEY_RCTRL },
		{ { sim::Controller::Impulse::FORCE, axes::UP, -1 }, KEY_LCTRL },
		
		{ { sim::Controller::Impulse::NUM_TYPES, axes::NUM_AXES, 0 }, KEY_MAX }
	};


	// Input mouse data

	struct InputMouseMapping
	{
		bool IsActive() const {
			return sys::IsButtonDown(button);
		}
		
		InpulseAffector affector;
		sys::MouseButton button;
	};

	InputMouseMapping buttons[] = 
	{
		{ { sim::Controller::Impulse::FORCE, axes::UP, -1 }, sys::BUTTON_LEFT },
		{ { sim::Controller::Impulse::FORCE, axes::UP, +1 }, sys::BUTTON_RIGHT },
		
		{ { sim::Controller::Impulse::NUM_TYPES, axes::NUM_AXES, 0 }, sys::BUTTON_MAX }
	};

}


sim::UserInput::UserInput()
{ 
}

sim::Controller::Impulse sim::UserInput::GetImpulse()
{
	Impulse impulse;
	
	// Early out for screen capture.
	/*if (capture) {
		impulse.factors[FORCE][2] = 1;
		return;
	}*/
	
	// keyboard
	MapInputs(keys, impulse);
	
	// mouse buttons
	MapInputs(buttons, impulse);
	
	// mouse movement
	Vector2i delta;
	Vector2i window_size = sys::GetWindowSize();
	Vector2i center(window_size.x >> 1, window_size.y >> 1);
#if __APPLE__ && 0
	CGGetLastMouseDelta (& delta.x, & delta.y);
#else
	Vector2i const & cursor(sys::GetMousePosition());
	delta = (cursor - center);
#endif
	Vector2f mouse_input = Vector2f(delta) * 0.3f;
	if (Length(mouse_input) > 0) {
		impulse.factors[sim::Controller::Impulse::TORQUE][axes::UP] -= mouse_input.x;
		impulse.factors[sim::Controller::Impulse::TORQUE][axes::RIGHT] -= mouse_input.y;
	}
	
	sys::SetMousePosition(Vector2i(center.x, center.y));
	return impulse;
}
