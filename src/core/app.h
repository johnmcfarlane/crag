//
//  app.h
//  crag
//
//  Created by John on 1/24/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "geom/Vector2.h"


namespace app
{
	// init
	bool Init(Vector2i resolution, bool full_screen, char const * title, char const * program_path);
	void Deinit();
	
	char const * GetProgramPath();
	bool LoadFile(char const * filename, std::vector<char> & buffer);
	
	// input
	bool IsKeyDown(SDL_Scancode key_code);
	bool IsButtonDown(int mouse_button);
	
	// window
	SDL_Window & GetWindow();
	Vector2i GetWindowSize();
	
	// called from main thread
	bool GetEvent(SDL_Event & event);
	
	// thread safe
	void PushEvent(SDL_Event const & event);
	bool PopEvent(SDL_Event & event);
	
	bool HasFocus();
	
	// time
	Time GetTime();
}
