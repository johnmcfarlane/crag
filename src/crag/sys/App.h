/*
 *  App.h
 *  Crag
 *
 *  Created by John on 1/24/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "geom/Vector2.h"

#include "glpp/gl.h"


#define KEY_0 SDL_SCANCODE_0
#define KEY_1 SDL_SCANCODE_1
#define KEY_9 SDL_SCANCODE_9
#define KEY_LEFT SDL_SCANCODE_LEFT
#define KEY_RIGHT SDL_SCANCODE_RIGHT
#define KEY_UP SDL_SCANCODE_UP
#define KEY_DOWN SDL_SCANCODE_DOWN
#define KEY_INSERT SDL_SCANCODE_INSERT
#define KEY_DELETE SDL_SCANCODE_DELETE
#define KEY_HOME SDL_SCANCODE_HOME
#define KEY_END SDL_SCANCODE_END
#define KEY_PAGEUP SDL_SCANCODE_PAGEUP
#define KEY_PAGEDOWN SDL_SCANCODE_PAGEDOWN
#define KEY_A SDL_SCANCODE_A
#define KEY_C SDL_SCANCODE_C
#define KEY_D SDL_SCANCODE_D
#define KEY_E SDL_SCANCODE_E
#define KEY_F SDL_SCANCODE_F
#define KEY_G SDL_SCANCODE_G
#define KEY_I SDL_SCANCODE_I
#define KEY_K SDL_SCANCODE_K
#define KEY_L SDL_SCANCODE_L
#define KEY_M SDL_SCANCODE_M
#define KEY_O SDL_SCANCODE_O
#define KEY_P SDL_SCANCODE_P
#define KEY_Q SDL_SCANCODE_Q
#define KEY_W SDL_SCANCODE_W
#define KEY_S SDL_SCANCODE_S
#define KEY_COMMA SDL_SCANCODE_COMMA
#define KEY_PERIOD SDL_SCANCODE_PERIOD
#define KEY_RETURN SDL_SCANCODE_RETURN
#define KEY_ESCAPE SDL_SCANCODE_ESCAPE
#define KEY_SPACE SDL_SCANCODE_SPACE
#define KEY_RSHIFT SDL_SCANCODE_RSHIFT
#define KEY_LSHIFT SDL_SCANCODE_LSHIFT
#define KEY_RCTRL SDL_SCANCODE_RCTRL
#define KEY_LCTRL SDL_SCANCODE_LCTRL
#define KEY_RALT SDL_SCANCODE_RALT
#define KEY_LALT SDL_SCANCODE_LALT


namespace sys
{
	// types
	typedef SDL_Event Event;
	typedef SDLKey KeyCode;
	typedef double TimeType;	// 1 unit = 1 second
	
	enum MouseButton
	{
		BUTTON_LEFT,
		BUTTON_RIGHT,
		BUTTON_MAX
	};
	
	// init
	bool Init(Vector2i resolution, bool full_screen, char const * title, char const * program_path);
	void Deinit();
	
	char const * GetProgramPath();

	bool InitGl();
	void DeinitGl();
	
	// input
	bool IsKeyDown(KeyCode key_code);
	bool IsButtonDown(MouseButton mouse_button);
	
	// window
	Vector2i GetWindowSize();
	//SDL_Surface & GetVideoSurface();
	void SwapBuffers();
	
	bool GetEvent(Event & event, bool block);
	bool HasFocus();
	
	// time
	TimeType GetTime();
}
