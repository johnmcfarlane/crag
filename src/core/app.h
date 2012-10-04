//
//  app.h
//  crag
//
//  Created by John on 1/24/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace app
{
	// init
	bool Init(geom::Vector2i resolution, bool full_screen, char const * title, char const * program_path);
	void Deinit();
	
	char const * GetProgramPath();
	bool LoadFile(char const * filename, std::vector<char> & buffer);
	
	// input
	bool IsKeyDown(SDL_Scancode key_code);
	bool IsButtonDown(int mouse_button);
	
	// window
	SDL_Window & GetWindow();
	geom::Vector2i GetWindowSize();
	
	// called from main thread
	void GetEvent(SDL_Event & event);
	
	// time
	core::Time GetTime();
}
