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

#include "glpp/glpp.h"

#if defined(__APPLE__)
#include <CoreFoundation/CFDate.h>
#endif


// defined in Renderer.cpp
CONFIG_DECLARE (multisample, bool);


namespace 
{
	bool _has_focus = true;
	bool _relative_mouse_mode = true;
	
	SDL_Window * window = nullptr;
		
#if defined(WIN32)
	Time inv_query_performance_frequency = 0;
#endif
	
	char const * _program_path;

	void SetFocus(bool has_focus)
	{
		_has_focus = has_focus;
	}
}


bool app::Init(Vector2i resolution, bool full_screen, char const * title, char const * program_path)
{
#if defined(WIN32)
	LARGE_INTEGER query_performance_frequency;
	if (QueryPerformanceFrequency(& query_performance_frequency) == FALSE || query_performance_frequency.QuadPart == 0)
	{
		std::cerr << "Failed to read QueryPerformanceFrequency." << std::endl;
		return false;
	}
	
	inv_query_performance_frequency = 1. / query_performance_frequency.QuadPart;
#endif

	// Initialize SDL.
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		DEBUG_BREAK_SDL();
		return false;
	}
	
	Assert(window == nullptr);

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
	
	AssertErrno();
	window = SDL_CreateWindow("Crag", 
							  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
							  resolution.x, resolution.y, 
							  flags);
	errno = 0;
	
	if (window == 0)
	{
		DEBUG_BREAK_SDL();
		return false;
	}
	
	SetFocus(true);
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
	Assert(window != nullptr);

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
	AssertErrno();
	
	FILE * source = fopen(filename, "r");
	if (source == nullptr)
	{
		Assert(errno == ENOENT);
		std::cerr << DEBUG_PRINT_SOURCE << ": file not found \"" << filename << "\"" << std::endl;
		errno = 0;
		return false;
	}
	
	fseek(source, 0, SEEK_END);
	size_t length = ftell(source);
	fseek(source, 0, SEEK_SET);

	buffer.resize(length);
	
	fread(& buffer[0], 1, length, source);

	fclose(source);
	
	AssertErrno();
	
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
	
	Assert(false);
	return false;
}

bool app::IsButtonDown(int mouse_button)
{
	Uint8 mouse_state = SDL_GetMouseState(NULL, NULL);
	return (mouse_state & SDL_BUTTON(mouse_button)) != 0;
}

SDL_Window & app::GetWindow()
{
	return ref(window);
}

Vector2i app::GetWindowSize()
{
	Vector2i window_size;	
	SDL_GetWindowSize(window, & window_size.x, & window_size.y);
	return window_size;
}

bool app::GetEvent(SDL_Event & event, bool block)
{
	AssertErrno();
	bool has_event = (block ? SDL_WaitEvent : SDL_PollEvent)(&event);
	errno = 0;
	
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
					SetFocus(true);
					break;
					
				case SDL_WINDOWEVENT_FOCUS_LOST:
					SetFocus(false);
					break;
			}
			break;
			
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			// event.button.button
			break;

		case SDL_MOUSEMOTION:
			{
				if (_relative_mouse_mode)
				{
					// if relative mouse mode is working, 
					// let ScriptThread take care of this properly
					break;
				}

				if (! _has_focus)
				{
					return false;
				}

				// intercept message and fake relative mouse movement correctly.
				Vector2i window_size;
				SDL_GetWindowSize(window, & window_size.x, & window_size.y);
				Vector2i center(window_size.x >> 1, window_size.y >> 1);
				Vector2i cursor;
				SDL_GetMouseState(& cursor.x, & cursor.y);
				SDL_WarpMouseInWindow(window, center.x, center.y);
				Vector2i delta = (cursor - center);
				if (delta.x == 0 && delta.y == 0)
				{
					return false;
				}

				// fake a mouse motion event
				event.type = SDL_MOUSEMOTION;
				event.motion.x = cursor.x;
				event.motion.y = cursor.y;
				event.motion.xrel = delta.x;
				event.motion.yrel = delta.y;
				return true;

				/*Vector2f mouse_input = Vector2f(delta) * 0.3f;
				if (Length(mouse_input) > 0) {
						impulse.factors[sim::Controller::Impulse::TORQUE][axes::UP] -= mouse_input.x;
						impulse.factors[sim::Controller::Impulse::TORQUE][axes::RIGHT] -= mouse_input.y;
				}*/
				// Note: OS X API provides CGGetLastMouseDelta (& delta.x, & delta.y);
			}
		break;
	}
	
	return true;
}

bool app::HasFocus()
{
	return _has_focus;
}

Time app::GetTime()
{
#if defined(__APPLE__)
	return CFAbsoluteTimeGetCurrent ();
#elif defined(WIN32)
	LARGE_INTEGER performance_count;
	if (QueryPerformanceCounter(& performance_count) == FALSE)
	{
		Assert(false);
	}
	return inv_query_performance_frequency * performance_count.QuadPart;
#else
	// Might want to try CLOCK_MONOTONIC clock using POSIX clock_gettime.
	return .001 * SDL_GetTicks();
#endif
}
