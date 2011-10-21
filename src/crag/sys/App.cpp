/*
 *  App.cpp
 *  Crag
 *
 *  Created by John on 1/24/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "App.h"

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
	
	bool button_down [sys::BUTTON_MAX];
	
	SDL_Window * window = nullptr;
	SDL_GLContext context = nullptr;
		
#if defined(WIN32)
	sys::TimeType inv_query_performance_frequency = 0;
#endif
	
	char const * _program_path;

	void SetFocus(bool has_focus)
	{
		_has_focus = has_focus;
	}
	
	bool InitGlew()
	{
#if defined(GLEW_STATIC)
		GLenum glew_err = glewInit();
		if (glew_err != GLEW_OK)
		{
			std::cerr << "GLEW Error: " << glewGetErrorString(glew_err) << std::endl;
			return false;
		}
		
		std::cout << "GLEW Version: " << glewGetString(GLEW_VERSION) << std::endl;

		if (! GLEW_ARB_vertex_buffer_object)
		{
			std::cerr << "GL implementation doesn't support vertex buffers." << std::endl;
			return false;
		}
#endif
		
		return true;
	}
}


bool sys::Init(Vector2i resolution, bool full_screen, char const * title, char const * program_path)
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
		ReportSdlError("Failed to initialize SDL");
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
			ReportSdlError("Failed to get desktop display mode.");
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
		ReportSdlError("Failed to initialize display.");
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

	ZeroObject(button_down);
	
	_program_path = program_path;
	
	return true;
}

void sys::Deinit()
{
	Assert(window != nullptr);

	SDL_DestroyWindow(window);
	window = nullptr;
	
	SDL_Quit();

}

char const * sys::GetProgramPath()
{
	return _program_path;
}

void sys::ReportSdlError(char const * message)
{
	std::cerr << message << ": " << SDL_GetError() << std::endl;
}

bool sys::GlInit()
{
	Assert(context == nullptr);
	context = SDL_GL_CreateContext(window);
	
	if (SDL_GL_MakeCurrent(window, context) != 0)
	{
		ReportSdlError("Failed to get GL context");
		return false;
	}

	if (! InitGlew())
	{
		return false;
	}

	return true;
}

void sys::GlDeinit()
{
	Assert(context != nullptr);

	SDL_GL_DeleteContext(context);
	context = nullptr;
}

bool sys::GlSupportsFences()
{
#if defined(__APPLE__)
	return true;
#else
	return GLEW_NV_fence != GL_FALSE;
#endif
}

bool sys::IsKeyDown(SDL_Scancode key_code)
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

bool sys::IsButtonDown(MouseButton mouse_button)
{
	return button_down[mouse_button];
}

Vector2i sys::GetWindowSize()
{
	Vector2i window_size;	
	SDL_GetWindowSize(window, & window_size.x, & window_size.y);
	return window_size;
}

Vector2i sys::GetWindowPosition()
{
	Vector2i window_position;	
	SDL_GetWindowPosition(window, & window_position.x, & window_position.y);
	return window_position;
}

void sys::SwapBuffers()
{
	SDL_GL_SwapWindow(window);
}

bool sys::GetEvent(Event & event, bool block)
{
	if ((block ? SDL_WaitEvent : SDL_PollEvent)(&event) <= 0)
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
			button_down [event.button.button] = true;
			break;
			
		case SDL_MOUSEBUTTONUP:
			button_down [event.button.button] = false;
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
				Vector2i window_size(sys::GetWindowSize());
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

bool sys::HasFocus()
{
	return _has_focus;
}

sys::TimeType sys::GetTime()
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
