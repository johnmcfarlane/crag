/*
 *  AppSdl.h
 *  Crag
 *
 *  Created by John on Apr 21, 2010.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


#if defined WELL_THIS_DID_SEEM_TO_WORK_UNTIL_I_TRIED_IT_ON_VC10Ex
#include <SDL/SDL_keyboard.h>
#include <SDL/SDL_events.h>
#endif

#define KEY_0 SDLK_0
#define KEY_1 SDLK_1
#define KEY_9 SDLK_9
#define KEY_LEFT SDLK_LEFT
#define KEY_RIGHT SDLK_RIGHT
#define KEY_UP SDLK_UP
#define KEY_DOWN SDLK_DOWN
#define KEY_INSERT SDLK_INSERT
#define KEY_DELETE SDLK_DELETE
#define KEY_HOME SDLK_HOME
#define KEY_END SDLK_END
#define KEY_PAGEUP SDLK_PAGEUP
#define KEY_PAGEDOWN SDLK_PAGEDOWN
#define KEY_A SDLK_a
#define KEY_C SDLK_c
#define KEY_D SDLK_d
#define KEY_E SDLK_e
#define KEY_F SDLK_f
#define KEY_G SDLK_g
#define KEY_I SDLK_i
#define KEY_L SDLK_l
#define KEY_M SDLK_m
#define KEY_O SDLK_o
#define KEY_P SDLK_p
#define KEY_Q SDLK_q
#define KEY_W SDLK_w
#define KEY_S SDLK_s
#define KEY_RETURN SDLK_RETURN
#define KEY_ESCAPE SDLK_ESCAPE
#define KEY_SPACE SDLK_SPACE
#define KEY_RSHIFT SDLK_RSHIFT
#define KEY_LSHIFT SDLK_LSHIFT
#define KEY_RCTRL SDLK_RCTRL
#define KEY_LCTRL SDLK_LCTRL
#define KEY_RALT SDLK_RALT
#define KEY_LALT SDLK_LALT
#define KEY_MAX SDLK_LAST

namespace app
{
	typedef SDL_Event Event;
	typedef SDLKey KeyCode;
	typedef int TimeType;

	enum MouseButton
	{
		BUTTON_LEFT,
		BUTTON_RIGHT,
		BUTTON_MAX
	};

	bool Init(Vector2i const & resolution, bool full_screen);

	bool IsKeyDown(KeyCode key_code);
	bool IsMouseDown(MouseButton mouse_button);

	SDL_Surface & GetVideoSurface();
}

