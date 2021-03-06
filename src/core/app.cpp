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

#if defined(CRAG_OS_WINDOWS)
#include "core/windows.h"
#endif

#include <SDL_image.h>

namespace 
{
	SDL_Window * _window = nullptr;
	SDL_GLContext _context = nullptr;

	SDL_DisplayMode _desktop_display_mode;
	SDL_DisplayMode _current_display_mode;

	geom::Vector2i _resolution;
	bool _full_screen;
	char const * _title;

	int _num_keys = -1;
	Uint8 const * _key_state_map = nullptr;
	
	bool InitWindow()
	{
		ASSERT(! _window);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	
#if defined(CRAG_GL)
#if defined(CRAG_DEBUG)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

#if defined(CRAG_GLES)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

		int flags = 0;

#if defined(CRAG_GL)
		flags |= SDL_WINDOW_OPENGL;
#endif

		if (_full_screen)
		{
			_resolution.x = _desktop_display_mode.w;
			_resolution.y = _desktop_display_mode.h;
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
	
		DEBUG_MESSAGE("Creating window %d,%d", _resolution.x, _resolution.y);

		_window = SDL_CreateWindow(
			_title, 
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
			_resolution.x, _resolution.y, 
			flags);
	
		if (! _window)
		{
			CRAG_REPORT_SDL_ERROR();
			return false;
		}
	
		if (SDL_GetCurrentDisplayMode(0, & _current_display_mode))
		{
			CRAG_REPORT_SDL_ERROR();
			return false;
		}
	
		return true;
	}

	void DeinitWindow()
	{
		ASSERT(_window);
		SDL_DestroyWindow(_window);
		_window = nullptr;
	}

	bool InitContext()
	{
		ASSERT(_window);
		ASSERT(! _context);
		_context = SDL_GL_CreateContext(_window);
		if (! _context)
		{
			CRAG_REPORT_SDL_ERROR();
			return false;
		}

		return true;
	}

	void DeinitContext()
	{
		ASSERT(_context);
		SDL_GL_DeleteContext(_context);
		_context = nullptr;
	}
}

void app::Quit()
{
	SDL_Event event;
	event.quit = { SDL_QUIT, 0 };

	if (! SDL_PushEvent(& event))
	{
		CRAG_REPORT_SDL_ERROR();
	}
}

bool app::Init(geom::Vector2i resolution, bool full_screen, char const * title)
{
	_resolution = resolution;
	_full_screen = full_screen;
	_title = title;

	// Initialize SDL.
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		CRAG_REPORT_SDL_ERROR();
		return false;
	}
	
	if(SDL_GetDesktopDisplayMode(0, & _desktop_display_mode) != 0)
	{
		CRAG_REPORT_SDL_ERROR();
		return false;
	}
	
	_current_display_mode = _desktop_display_mode;

	if (! InitWindow())
	{
		return false;
	}

	// get pointer to keyboard state map
	_key_state_map = SDL_GetKeyboardState(& _num_keys);

	return true;
}

void app::Deinit()
{
	DeinitWindow();
	
	IMG_Quit();

	SDL_Quit();

	CRAG_DEBUG_CHECK_MEMORY();
}

bool app::InitGfx()
{
	if (! InitContext())
	{
		return false;
	}
	
	return true;
}

void app::DeinitGfx()
{
	DeinitContext();
}

char const * app::GetAssetPath(char const * filepath)
{
#if defined(CRAG_OS_ANDROID)
	static constexpr char prefix[] = "assets/";
	static constexpr std::size_t prefix_size = std::extent<decltype(prefix)>::value - 1;
	ASSERT(strncmp(filepath, prefix, prefix_size) == 0);
	
	return filepath + prefix_size;
#else
	return filepath;
#endif
}

std::string app::GetStatePath(std::string const & filepath)
{
#if defined(CRAG_OS_ANDROID)
	return std::string(SDL_AndroidGetInternalStoragePath()) + filepath;
#else
	return filepath;
#endif
}

app::FileResource app::LoadFile(char const * filename, app::FileType file_type, bool null_terminate)
{
	ASSERT(filename);

	// open file
	SDL_RWops * source;
	switch (file_type)
	{
		case app::FileType::asset:
			source = SDL_RWFromFile(app::GetAssetPath(filename), "rb");
			break;

		case app::FileType::state:
			source = SDL_RWFromFile(app::GetStatePath(filename).c_str(), "rb");
			break;

		default:
			DEBUG_BREAK("invalid enum value, %d", static_cast<int>(file_type));
			return FileResource();
	}

	if (source == nullptr)
	{
		DEBUG_MESSAGE("LoadFile: %s", SDL_GetError());
		return FileResource();
	}
	
	// determine length
	auto length = static_cast<std::size_t>(SDL_RWseek(source, 0, SEEK_END));
	SDL_RWseek(source, 0, SEEK_SET);

	// prepare buffer
	std::size_t size = length + null_terminate;
	FileResource buffer(size);
	if (buffer.size() != size)
	{
		DEBUG_MESSAGE("failed to allocate " SIZE_T_FORMAT_SPEC " bytes file, '%s':", size, filename);
		return FileResource();
	}
	
	// read file
	auto read = SDL_RWread(source, buffer.data(), 1, length);
	if (read != length)
	{
		DEBUG_MESSAGE("only read " SIZE_T_FORMAT_SPEC " bytes of " SIZE_T_FORMAT_SPEC " byte file, '%s':", read, length, filename);
		CRAG_REPORT_SDL_ERROR();
		return FileResource();
	}

	// close file
	if (SDL_RWclose(source) != 0)
	{
		DEBUG_MESSAGE("failed to close file, '%s':", filename);
		CRAG_REPORT_SDL_ERROR();
		return FileResource();
	}
	
	// append terminator
	if (null_terminate)
	{
		buffer[length] = '\0';
		
		// If this happens, it probably means that a null terminator was read from the file.
		ASSERT(strlen(buffer.data()) == length);
	}
	
	// success!
	return buffer;
}

bool app::SaveFile(char const * filename, app::FileType file_type, app::FileResource const & buffer)
{
	return SaveFile(filename, file_type, static_cast<void const *>(buffer.data()), buffer.size());
}

bool app::SaveFile(char const * filename, app::FileType file_type, void const * data, std::size_t size)
{
	ASSERT(filename);

	// open file
	SDL_RWops * destination;
	switch (file_type)
	{
		case app::FileType::asset:
			destination = SDL_RWFromFile(app::GetAssetPath(filename), "wb");
			break;

		case app::FileType::state:
			destination = SDL_RWFromFile(app::GetStatePath(filename).c_str(), "wb");
			break;

		default:
			DEBUG_BREAK("invalid enum value, %d", static_cast<int>(file_type));
			return false;
	}

	if (destination == nullptr)
	{
		DEBUG_MESSAGE("SaveFile: %s", SDL_GetError());
		return false;
	}

	// write file
	auto length = size;
	auto written = SDL_RWwrite(destination, data, 1, length);
	if (written != length)
	{
		DEBUG_MESSAGE("only wrote " SIZE_T_FORMAT_SPEC " bytes of " SIZE_T_FORMAT_SPEC " byte file, '%s':", written, length, filename);
		CRAG_REPORT_SDL_ERROR();
		return false;
	}

	// close file
	if (SDL_RWclose(destination) != 0)
	{
		DEBUG_MESSAGE("failed to close file, '%s':", filename);
		CRAG_REPORT_SDL_ERROR();
		return false;
	}

	// success!
	return true;
}

void app::Beep()
{
#if defined(CRAG_OS_WINDOWS)
	MessageBeep(MB_ICONINFORMATION);
#endif
}

bool app::IsKeyDown(SDL_Scancode key_code)
{
	CRAG_VERIFY_OP(key_code, >=, 0);
	CRAG_VERIFY_OP(key_code, <, _num_keys);
	CRAG_VERIFY_EQUAL(_key_state_map, SDL_GetKeyboardState(nullptr));
	
	return _key_state_map[key_code] != 0;
}

bool app::IsButtonDown(int mouse_button)
{
	auto mouse_state = SDL_GetMouseState(nullptr, nullptr);
	return (mouse_state & SDL_BUTTON(mouse_button)) != 0;
}

geom::Vector2i app::GetResolution()
{
	geom::Vector2i window_size;	
	SDL_GetWindowSize(_window, & window_size.x, & window_size.y);
	return window_size;
}

int app::GetRefreshRate()
{
	return _current_display_mode.refresh_rate;
}

void app::SwapBuffers()
{
	SDL_GL_SwapWindow(_window);
}

bool app::GetEvent(SDL_Event & event, bool block)
{
	// get or block
	if (block)
	{
		if (! SDL_WaitEvent(& event))
		{
			CRAG_REPORT_SDL_ERROR();
			return false;
		}
		
		return true;
	}
	
	// get or return
	bool has_event = SDL_PollEvent(& event) != 0;
	
	
	return has_event;
}

core::Time app::GetTime()
{
	auto now = std::chrono::steady_clock::now();
	auto duration = now.time_since_epoch();
	core::Time seconds = core::DurationToSeconds(duration);
	return seconds;
}
