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


namespace app
{
	// init
	bool Init(Vector2i resolution, bool full_screen, char const * title, char const * program_path);
	void Deinit();
	
	char const * GetProgramPath();
	
	// input
	bool IsKeyDown(SDL_Scancode key_code);
	bool IsButtonDown(int mouse_button);
	
	// window
	SDL_Window & GetWindow();
	Vector2i GetWindowSize();
	
	bool GetEvent(SDL_Event & event, bool block);
	bool HasFocus();
	
	// time
	Time GetTime();
}
