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

#if defined(WIN32)
#include "core/windows.h"
#endif

namespace 
{
	bool _has_focus = true;
	
	SDL_Window * window = nullptr;
	SDL_Renderer * renderer = nullptr;	// this isn't needed for Win32/Android
	SDL_GLContext context = nullptr;

	int refresh_rate = -1;

	int num_keys = -1;
	Uint8 const * key_state_map = nullptr;
}

bool app::Init(geom::Vector2i resolution, bool full_screen, char const * title)
{
	// Initialize SDL.
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		DEBUG_BREAK_SDL();
		return false;
	}
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

#if defined(CRAG_USE_GL)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

#if defined(CRAG_USE_GLES)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

	int flags = SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_OPENGL;

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
		flags |= SDL_WINDOW_FULLSCREEN;	// TODO: investigate SDL_WINDOW_FULLSCREEN_DESKTOP
	}
	
	DEBUG_MESSAGE("Creating window %d,%d", resolution.x, resolution.y);

	ASSERT(window == nullptr);
	window = SDL_CreateWindow(
		title, 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		resolution.x, resolution.y, 
		flags);
	
	if (window == 0)
	{
		DEBUG_BREAK_SDL();
		return false;
	}
	
	_has_focus = true;

	// get pointer to keyboard state map
	key_state_map = SDL_GetKeyboardState(& num_keys);

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

void app::Quit()
{
	SDL_Event event;
	event.quit = { SDL_QUIT, 0 };

	if (! SDL_PushEvent(& event))
	{
		DEBUG_BREAK_SDL();
	}
}

bool app::InitContext()
{
	ASSERT(window != nullptr);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE*/);
	if (renderer == nullptr)
	{
		DEBUG_MESSAGE("Failed to create renderer: \"%s\"", SDL_GetError());	
		return false;
	}
	
	context = SDL_GL_CreateContext(window);
	if (context == nullptr)
	{
		DEBUG_BREAK_SDL();
		return false;
	}

	return true;
}

void app::DeinitContext()
{
	ASSERT(context != nullptr);
	SDL_GL_DeleteContext(context);
	context = nullptr;

	ASSERT(renderer != nullptr);
	SDL_DestroyRenderer(renderer);
	renderer = nullptr;
}

char const * app::GetFullPath(char const * filepath)
{
#if defined(__ANDROID__)
	static constexpr char prefix[] = "assets/";
	static constexpr std::size_t prefix_size = std::extent<decltype(prefix)>::value - 1;
	ASSERT(strncmp(filepath, prefix, prefix_size) == 0);
	
	return filepath + prefix_size;
#else
	return filepath;
#endif
}

app::FileResource app::LoadFile(char const * filename, bool null_terminate)
{
	// open file
	SDL_RWops * source = SDL_RWFromFile(app::GetFullPath(filename), "rb");
	if (source == nullptr)
	{
		DEBUG_MESSAGE("failed to open file, '%s':", filename);
		DEBUG_BREAK_SDL();
		return FileResource();
	}
	
	// determine length
	auto length = static_cast<std::size_t>(SDL_RWseek(source, 0, SEEK_END));
	SDL_RWseek(source, 0, SEEK_SET);

	// prepare buffer
	std::size_t size = length + null_terminate;
	FileResource buffer(new app::FileResource::element_type(size));
	if (! buffer || buffer->size() != size)
	{
		DEBUG_MESSAGE("failed to allocated " SIZE_T_FORMAT_SPEC " bytes file, '%s':", size, filename);
		return FileResource();
	}
	
	// read file
	auto read = SDL_RWread(source, & buffer->front(), 1, length);
	if (read != length)
	{
		DEBUG_MESSAGE("only read " SIZE_T_FORMAT_SPEC " bytes of " SIZE_T_FORMAT_SPEC " byte file, '%s':", read, length, filename);
		DEBUG_BREAK_SDL();
		return FileResource();
	}

	// close file
	if (SDL_RWclose(source) != 0)
	{
		DEBUG_MESSAGE("failed to close file, '%s':", filename);
		DEBUG_BREAK_SDL();
		return FileResource();
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

void app::Beep()
{
#if defined(WIN32)
	MessageBeep(MB_ICONINFORMATION);
#endif
}

bool app::IsKeyDown(SDL_Scancode key_code)
{
	CRAG_VERIFY_OP(key_code, >=, 0);
	CRAG_VERIFY_OP(key_code, <, num_keys);
	CRAG_VERIFY_EQUAL(key_state_map, SDL_GetKeyboardState(nullptr));
	
	return key_state_map[key_code] != 0;
}

bool app::IsButtonDown(int mouse_button)
{
	auto mouse_state = SDL_GetMouseState(nullptr, nullptr);
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

bool app::GetEvent(SDL_Event & event, bool block)
{
	// get or block
	if (block)
	{
		if (! SDL_WaitEvent(& event))
		{
			DEBUG_BREAK_SDL();
			return false;
		}
		
		return true;
	}
	
	// get or return
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
