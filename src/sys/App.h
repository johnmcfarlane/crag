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


namespace sys
{
	// types
	typedef SDL_Event Event;
	typedef double Time;	// 1 unit = 1 second
	
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
	
	bool GlInit();
	void GlDeinit();
	bool GlSupportsFences();
	
	// input
	bool IsKeyDown(SDL_Scancode key_code);
	bool IsButtonDown(MouseButton mouse_button);
	
	// window
	Vector2i GetWindowSize();
	Vector2i GetWindowPosition();
	void SwapBuffers();
	
	bool GetEvent(Event & event, bool block);
	bool HasFocus();
	
	// time
	Time GetTime();
}
