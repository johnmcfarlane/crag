//
//  app.h
//  crag
//
//  Created by John on 1/24/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#if defined(__ANDROID__)
#define CRAG_USE_TOUCH
#else
#define CRAG_USE_MOUSE	// ... and keyboard
#endif

namespace app
{
	// general init/deinit
	bool Init(geom::Vector2i resolution, bool full_screen, char const * title);
	void Deinit();

	// render thread-specific init/deinit
	bool InitContext();
	void DeinitContext();
	
	// file access
	typedef std::shared_ptr<std::vector<char>> FileResource;
	char const * GetFullPath(char const * filepath);
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
