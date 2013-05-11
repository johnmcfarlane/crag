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
	// general init/deinit
	bool Init(geom::Vector2i resolution, bool full_screen, char const * title);
	void Deinit();

	// render thread-specific init/deinit
	SDL_GLContext InitContext();
	void DeinitContext(SDL_GLContext context);
	
	// file access
	typedef std::shared_ptr<std::vector<char>> FileResource;
	FileResource LoadFile(char const * filename, bool null_terminate = false);
	
	// misc
	void Beep();
	
	// input
	bool IsKeyDown(SDL_Scancode key_code);
	bool IsButtonDown(int mouse_button);
	
	// video
	geom::Vector2i GetResolution();
	int GetRefreshRate();
	void SwapBuffers();

	// called from main thread
	bool GetEvent(SDL_Event & event);
	
	// time
	core::Time GetTime();
}
