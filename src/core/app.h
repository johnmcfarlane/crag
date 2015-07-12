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
	void Quit();

	// general init/deinit
	bool Init(geom::Vector2i resolution, bool full_screen, char const * title);
	void Deinit();
	
	// render thread-specific init/deinit
	bool InitGfx();
	void DeinitGfx();

	// file access
	typedef std::vector<char> FileResource;
	enum class FileType
	{
		asset,
		state
	};

	char const * GetAssetPath(char const * filepath);
	std::string GetStatePath(std::string const & filepath);

	FileResource LoadFile(char const * filename, FileType file_type, bool null_terminate = false);
	bool SaveFile(char const * filename, FileType file_type, FileResource const & buffer);
	bool SaveFile(char const * filename, FileType file_type, void const * data, std::size_t size);
	
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
	bool GetEvent(SDL_Event & event, bool block);
	
	// time
	core::Time GetTime();
}
