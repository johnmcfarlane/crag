//
//  app.cpp
//  crag
//
//  Created by John on 1/24/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "app.h"

#include "core/ConfigEntry.h"

#if ! defined(WIN32) && ! defined(__APPLE__)
#include <sys/resource.h>
#endif

using namespace app;

namespace 
{
	bool _has_focus = true;
	
	SDL_Window * window = nullptr;
	SDL_Renderer * renderer = nullptr;

	int refresh_rate = -1;

	FileResource::element_type * LoadFile(FILE & file, bool null_terminate)
	{
		// determine length
		if (fseek(& file, 0, SEEK_END) != 0)
		{
			return nullptr;
		}

		auto extent = ftell(& file);
		if (extent < 0)
		{
			return nullptr;
		}
		auto length = static_cast<std::size_t>(extent);

		if (fseek(& file, 0, SEEK_SET) != 0)
		{
			return nullptr;
		}

		// prepare buffer
		auto size = length + null_terminate;
		FileResource::element_type * buffer = new FileResource::element_type(size);
		if (! buffer || buffer->size() != size)
		{
			DEBUG_MESSAGE("failed to allocated file buffer of " SIZE_T_FORMAT_SPEC " bytes", size);
			return nullptr;
		}

		// read file
		auto read = fread(& buffer->front(), 1, length, & file);
		if (read != length)
		{
			DEBUG_MESSAGE("only read " SIZE_T_FORMAT_SPEC " bytes of " SIZE_T_FORMAT_SPEC " byte file", read, length);
			return nullptr;
		}

		// close file
		if (fclose(& file) != 0)
		{
			DEBUG_MESSAGE("failed to close file");
			return nullptr;
		}

		// append terminator
		if (null_terminate)
		{
			(* buffer)[length] = '\0';

			// If this happens, it probably means that a null terminator was read from the file.
			ASSERT(strlen(buffer->data()) == length);
		}

		// success!
		return buffer;
	}
}

bool app::Init(geom::Vector2i resolution, bool full_screen, char const * title)
{
#if ! defined(WIN32) && ! defined(NDEBUG)
	rlimit rlim;
	rlim.rlim_cur = rlim.rlim_max = 1024 * 1024;
	setrlimit(RLIMIT_CORE, &rlim);
#endif
	
	// Initialize SDL.
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		DEBUG_BREAK_SDL();
		return false;
	}
	
	ASSERT(window == nullptr);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	
	int flags = SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;

#if defined(CRAG_USE_MOUSE)
	flags |= SDL_WINDOW_MOUSE_FOCUS;
#endif
	
	// Get existing video info.
	SDL_DisplayMode desktop_display_mode;
	if(SDL_GetDesktopDisplayMode(0, & desktop_display_mode) != 0)
	{
		DEBUG_BREAK_SDL();
		return false;
	}

	refresh_rate = desktop_display_mode.refresh_rate;
	
	if (full_screen)
	{
		resolution.x = desktop_display_mode.w;
		resolution.y = desktop_display_mode.h;
		flags |= SDL_WINDOW_FULLSCREEN;
	}
	
	DEBUG_MESSAGE("Creating window %d,%d", resolution.x, resolution.y);
	window = SDL_CreateWindow(title, 
							  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
							  resolution.x, resolution.y, 
							  flags);
	
	if (window == 0)
	{
		DEBUG_BREAK_SDL();
		return false;
	}
	
	_has_focus = true;

	return true;
}

void app::Deinit()
{
	ASSERT(window != nullptr);

	SDL_DestroyWindow(window);
	window = nullptr;
	
	refresh_rate = -1;

	SDL_Quit();

	CRAG_DEBUG_CHECK_MEMORY();
}

bool app::InitContext()
{
	ASSERT(window != nullptr);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE*/);

	// currently, SDL_CreateRenderer triggers errno changes in X calls
	errno = 0;
	
	if (renderer == nullptr)
	{
		DEBUG_BREAK_SDL();
		return false;
	}
	
	return true;
}

void app::DeinitContext()
{
	ASSERT(renderer != nullptr);
	SDL_DestroyRenderer(renderer);
	renderer = nullptr;
}

FileResource app::LoadFile(char const * filename, bool null_terminate)
{
	AssertErrno();

#if defined(__ANDROID__)
	char path [PATH_MAX];
	snprintf(path, PATH_MAX, "%s/%s", SDL_AndroidGetInternalStoragePath(), filename);
	filename = path;
#endif

	// open file
	FILE * source = fopen(filename, "rb");
	if (source == nullptr)
	{
		DEBUG_MESSAGE("failed to open file, '%s':", filename);
		AssertErrno();
		return FileResource();
	}

	// load file
	auto buffer = ::LoadFile(* source, null_terminate);
	if (buffer == nullptr)
	{
		DEBUG_MESSAGE("error loading file, '%s':", filename);
		AssertErrno();
		return FileResource();
	}

	// return file contents
	return FileResource(buffer);
}

void app::Beep()
{
#if defined(WIN32)
	MessageBeep(MB_ICONINFORMATION);
#endif
}

bool app::IsKeyDown(SDL_Scancode key_code)
{
	if (key_code >= 0)
	{
		int num_keys;
		auto key_down = SDL_GetKeyboardState(& num_keys);
		
		if (key_code < num_keys)
		{
			return key_down[key_code] != false;
		}
	}
	
	ASSERT(false);
	return false;
}

bool app::IsButtonDown(int mouse_button)
{
	Uint8 mouse_state = SDL_GetMouseState(nullptr, nullptr);
	return (mouse_state & SDL_BUTTON(mouse_button)) != 0;
}

geom::Vector2i app::GetResolution()
{
	geom::Vector2i window_size;	
	SDL_GetWindowSize(window, & window_size.x, & window_size.y);
	return window_size;
}

int app::GetRefreshRate()
{
	return refresh_rate;
}

void app::SwapBuffers()
{
	SDL_GL_SwapWindow(window);
}

bool app::GetEvent(SDL_Event & event)
{
	bool has_event = SDL_PollEvent(& event) != 0;
	
	if (! has_event)
	{
		return false;
	}
	
	switch (event.type)
	{
		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					_has_focus = true;
					break;
					
				case SDL_WINDOWEVENT_FOCUS_LOST:
					_has_focus = false;
					break;
			}
			break;
	}

	return true;
}

core::Time app::GetTime()
{
	auto now = std::chrono::steady_clock::now();
	auto duration = now.time_since_epoch();
	core::Time seconds = core::DurationToSeconds(duration);
	return seconds;
}
