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
#include "X11/Xlib.h"
#endif

// defined in gfx::Engine.cpp
CONFIG_DECLARE (multisample, bool);


namespace 
{
	bool _has_focus = true;
	bool _relative_mouse_mode = true;
	
	SDL_Window * window = nullptr;
		
	char const * _program_path;
}

#include <sys/resource.h>
bool app::Init(geom::Vector2i resolution, bool full_screen, char const * title, char const * program_path)
{
	// X11 initialization
#if defined(XlibSpecificationRelease)
	Status xinit_status = XInitThreads();
	if (! xinit_status)
	{
		DEBUG_MESSAGE("Call to XInitThreads failed with return value, %d", xinit_status);
		return false;
	}
#endif
	
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
	
	if (multisample)
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);
	}
	
	int flags = SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
	
	if (full_screen)
	{
		// Get existing video info.
		SDL_DisplayMode desktop_display_mode;
		if(SDL_GetDesktopDisplayMode(0, & desktop_display_mode) != 0)
		{
			DEBUG_BREAK_SDL();
			return false;
		}
		
		resolution.x = desktop_display_mode.w;
		resolution.y = desktop_display_mode.h;
		flags |= SDL_WINDOW_FULLSCREEN;
	}
	
	window = SDL_CreateWindow("Crag", 
							  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
							  resolution.x, resolution.y, 
							  flags);
	
	if (window == 0)
	{
		DEBUG_BREAK_SDL();
		return false;
	}
	
	_has_focus = true;
	if (SDL_SetRelativeMouseMode(SDL_TRUE) == 0)
	{
		_relative_mouse_mode = true;
	}
	else
	{
		_relative_mouse_mode = false;
		SDL_SetWindowGrab(window, SDL_TRUE);
		SDL_ShowCursor(SDL_FALSE);
	}

	_program_path = program_path;
	
	return true;
}

void app::Deinit()
{
	ASSERT(window != nullptr);

	SDL_DestroyWindow(window);
	window = nullptr;
	
	SDL_Quit();

}

char const * app::GetProgramPath()
{
	return _program_path;
}

bool app::LoadFile(char const * filename, std::vector<char> & buffer)
{
	FILE * source = fopen(filename, "r");
	if (source == nullptr)
	{
		ASSERT(errno == ENOENT);
		ERROR_MESSAGE("file not found '%s'", filename);
		return false;
	}
	
	fseek(source, 0, SEEK_END);
	size_t length = ftell(source);
	fseek(source, 0, SEEK_SET);

	buffer.resize(length);
	
	size_t read = fread(& buffer[0], 1, length, source);
	if (read != length)
	{
		DEBUG_MESSAGE("error loading %s: length=%zd; read=%zd", filename, length, read);
		return false;
	}

	fclose(source);
	
	return true;
}

bool app::IsKeyDown(SDL_Scancode key_code)
{
	if (key_code >= 0)
	{
		int num_keys;
		Uint8 * key_down = SDL_GetKeyboardState(& num_keys);
		
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

SDL_Window & app::GetWindow()
{
	return ref(window);
}

geom::Vector2i app::GetWindowSize()
{
	geom::Vector2i window_size;	
	SDL_GetWindowSize(window, & window_size.x, & window_size.y);
	return window_size;
}

void app::GetEvent(SDL_Event & event)
{
	bool has_event = SDL_WaitEvent(& event) != 0;
	
	if (! has_event)
	{
		DEBUG_BREAK("SDL_WaitEvent failed");
		return;
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
			
		case SDL_MOUSEMOTION:
			{
				// TODO: Do we even use this any more?
				CleanMotionEvent(event.motion);
				break;
			}
		break;
	}
}

bool app::CleanMotionEvent(SDL_MouseMotionEvent & motion)
{
	if (_relative_mouse_mode)
	{
		// if relative mouse mode is working, 
		// let applet::Engine take care of this properly
		return true;
	}

	if (! _has_focus)
	{
		return false;
	}

	// intercept message and fake relative mouse movement correctly.
	geom::Vector2i window_size;
	SDL_GetWindowSize(window, & window_size.x, & window_size.y);
	geom::Vector2i center(window_size.x >> 1, window_size.y >> 1);
	geom::Vector2i cursor;
	SDL_GetMouseState(& cursor.x, & cursor.y);
	SDL_WarpMouseInWindow(window, center.x, center.y);
	geom::Vector2i delta = (cursor - center);
	if (delta.x == 0 && delta.y == 0)
	{
		return false;
	}

	// fake a mouse motion event
	motion.x = cursor.x;
	motion.y = cursor.y;
	motion.xrel = delta.x;
	motion.yrel = delta.y;
	
	return true;
}

core::Time app::GetTime()
{
	auto now = std::chrono::steady_clock::now();
	auto duration = now.time_since_epoch();
	core::Time seconds = core::DurationToSeconds(duration);
	return seconds;
}
