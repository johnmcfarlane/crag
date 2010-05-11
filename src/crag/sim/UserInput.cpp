/*
 *  UserInput.cpp
 *  Crag
 *
 *  Created by John on 1/1/10.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#include "pch.h"

#include "UserInput.h"
#include "core/Vector2.h"
#include "Space.h"

#include "app/App.h"


namespace ANONYMOUS {


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
	Space::Axis axis;	// [x/y/z]
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
	app::KeyCode key;
};

InputKeyMapping const keys[] = 
{
	// Arrow keys
	{ { sim::Controller::Impulse::FORCE, Space::RIGHT, -1 }, KEY_LEFT },
	{ { sim::Controller::Impulse::FORCE, Space::RIGHT, +1 }, KEY_RIGHT },
	{ { sim::Controller::Impulse::FORCE, Space::FORWARD, +1 }, KEY_UP },
	{ { sim::Controller::Impulse::FORCE, Space::FORWARD, -1 }, KEY_DOWN },
	
	// Above arrow keys (3x2)
	{ { sim::Controller::Impulse::TORQUE, Space::FORWARD, -1 }, KEY_INSERT },
	{ { sim::Controller::Impulse::FORCE, Space::RIGHT, -1 }, KEY_DELETE },
	{ { sim::Controller::Impulse::FORCE, Space::FORWARD, +1 }, KEY_HOME },
	{ { sim::Controller::Impulse::FORCE, Space::FORWARD, -1 }, KEY_END },
	{ { sim::Controller::Impulse::TORQUE, Space::FORWARD, +1 }, KEY_PAGEUP },
	{ { sim::Controller::Impulse::FORCE, Space::RIGHT, +1 }, KEY_PAGEDOWN },
	
	// FPS standard
	{ { sim::Controller::Impulse::TORQUE, Space::FORWARD, -1 }, KEY_Q },
	{ { sim::Controller::Impulse::FORCE, Space::RIGHT, -1 }, KEY_A },
	{ { sim::Controller::Impulse::FORCE, Space::FORWARD, +1 }, KEY_W },
	{ { sim::Controller::Impulse::FORCE, Space::FORWARD, -1 }, KEY_S },
	{ { sim::Controller::Impulse::TORQUE, Space::FORWARD, +1 }, KEY_E },
	{ { sim::Controller::Impulse::FORCE, Space::RIGHT, +1 }, KEY_D },
	
	// Lots of up/down options
	{ { sim::Controller::Impulse::FORCE, Space::UP, +1 }, KEY_SPACE },
	{ { sim::Controller::Impulse::FORCE, Space::UP, +1 }, KEY_RSHIFT },
	{ { sim::Controller::Impulse::FORCE, Space::UP, +1 }, KEY_LSHIFT },
	{ { sim::Controller::Impulse::FORCE, Space::UP, -1 }, KEY_RCTRL },
	{ { sim::Controller::Impulse::FORCE, Space::UP, -1 }, KEY_LCTRL },
	
	{ { sim::Controller::Impulse::NUM_TYPES, Space::NUM_AXES, 0 }, KEY_MAX }
};


// Input mouse data

struct InputMouseMapping
{
	bool IsActive() const {
		return app::IsButtonDown(button);
	}
	
	InpulseAffector affector;
	app::MouseButton button;
};

InputMouseMapping buttons[] = 
{
	{ { sim::Controller::Impulse::FORCE, Space::UP, -1 }, app::BUTTON_LEFT },
	{ { sim::Controller::Impulse::FORCE, Space::UP, +1 }, app::BUTTON_RIGHT },
	
	{ { sim::Controller::Impulse::NUM_TYPES, Space::NUM_AXES, 0 }, app::BUTTON_MAX }
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
	Vector2i window_size = app::GetWindowSize();
	Vector2i center(window_size.x >> 1, window_size.y >> 1);
#if __APPLE__ && 0
	CGGetLastMouseDelta (& delta.x, & delta.y);
#else
	Vector2i const & cursor(app::GetMousePosition());
	delta = (cursor - center);
#endif
	Vector2f mouse_input = Vector2f(delta) * 0.3f;
	if (Length(mouse_input) > 0) {
		impulse.factors[sim::Controller::Impulse::TORQUE][Space::UP] -= mouse_input.x;
		impulse.factors[sim::Controller::Impulse::TORQUE][Space::RIGHT] -= mouse_input.y;
	}
	
	app::SetMousePosition(Vector2i(center.x, center.y));
	return impulse;
}
