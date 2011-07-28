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

#include "ConfigEntry.h"

#include "glpp/glpp.h"

#if defined(__APPLE__)
#include <CoreFoundation/CFDate.h>
#endif


// defined in Renderer.cpp
CONFIG_DECLARE (multisample, bool);


namespace 
{
	bool has_focus = true;
	
	bool button_down [sys::BUTTON_MAX];
	
	Vector2i window_size;
	
	SDL_Window * window = nullptr;
	SDL_GLContext context = nullptr;
		
#if defined(WIN32)
	sys::TimeType inv_query_performance_frequency = 0;
#endif

	void SetFocus(bool gained_focus)
	{
		has_focus = gained_focus;
		SDL_SetWindowGrab(window, gained_focus ? SDL_TRUE : SDL_FALSE);
		SDL_WM_GrabInput(SDL_GRAB_ON);
		SDL_ShowCursor(has_focus ? SDL_DISABLE : SDL_ENABLE);
	}
	
	
	bool InitGl()
	{
#if defined(GLEW_STATIC )
		GLenum glew_err = glewInit();
		if (glew_err != GLEW_OK)
		{
			std::cout << "GLEW Error: " << glewGetErrorString(glew_err) << std::endl;
			return false;
		}
		
		std::cout << "GLEW Version: " << glewGetString(GLEW_VERSION) << std::endl;
#endif
		
		return true;
	}
}


bool sys::Init(Vector2i resolution, bool full_screen, char const * title)
{
#if defined(WIN32)
	LARGE_INTEGER query_performance_frequency;
	if (QueryPerformanceFrequency(& query_performance_frequency) == FALSE || query_performance_frequency.QuadPart == 0)
	{
		std::cerr << "Failed to read QueryPerformanceFrequency.\n";
		return false;
	}
	
	inv_query_performance_frequency = 1. / query_performance_frequency.QuadPart;
#endif

	// Initialize SDL.
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		std::cout << "Failed to initialize SDL: " << SDL_GetError();
		return false;
	}
	
	Assert(window == nullptr);
	Assert(context == nullptr);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	
	if (multisample)
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);
	}
	
	// Get existing video info.
	const SDL_VideoInfo* video_info = SDL_GetVideoInfo();
	if (video_info == nullptr)
	{
		std::cout << "Failed to get video info: " << SDL_GetError() << std::endl;
		return false;
	}
	
	int flags = SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
	
	if (full_screen)
	{
		resolution.x = video_info->current_w;
		resolution.y = video_info->current_h;
		flags |= SDL_WINDOW_FULLSCREEN;
	}
	
	window = SDL_CreateWindow("Crag", 
							  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
							  resolution.x, resolution.y, 
							  flags);
	
	context = SDL_GL_CreateContext(window);
	
	if (SDL_GL_SetSwapInterval(1))
	{
		std::cout << "Hardware doesn't support vsync: " << SDL_GetError() << std::endl;
		return false;
	}
	
	if (! InitGl())
	{
		return false;
	}
	
	SetFocus(true);
	SDL_WarpMouseInWindow(window, resolution.x >> 1, resolution.y >> 1);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	ZeroObject(button_down);
	
	window_size = resolution;
	
	return true;
}

void sys::Deinit()
{
	Assert(window != nullptr);
	Assert(context != nullptr);

	SDL_GL_DeleteContext(context);
	context = nullptr;

	SDL_DestroyWindow(window);
	window = nullptr;
	
	SDL_Quit();

}

bool sys::IsKeyDown(KeyCode key_code)
{
	if (key_code >= 0)
	{
		int num_keys;
		Uint8 * key_down = SDL_GetKeyboardState(& num_keys);
		
		if (key_code < num_keys)
		{
			return key_down[key_code];
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
	return window_size;
}

void sys::MakeCurrent()
{
	if (SDL_GL_MakeCurrent(window, context) != 0)
	{
		Assert(false);
	}
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
		case SDL_VIDEORESIZE:
			window_size.x = event.resize.w;
			window_size.y = event.resize.h;
			break;
			
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
	}
	
	return true;
}

bool sys::HasFocus()
{
	return has_focus;
}

sys::TimeType sys::GetTime()
{
#if defined(__APPLE__)
	return CFAbsoluteTimeGetCurrent ();
#elif defined(WIN32) && 0
	// TODO: Test this
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
