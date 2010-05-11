/*
 *  AppSdl.cpp
 *  Crag
 *
 *  Created by John on Apr 21, 2010.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#include "pch.h"

#include "app/App.h"

#include "core/memory.h"
#include "core/Vector2.h"

#include <SDL/SDL.h>


namespace app
{
	namespace
	{
		bool has_focus = true;

		bool key_down [KEY_MAX];
		bool button_down [BUTTON_MAX];

		Vector2i mouse_position;
		Vector2i window_size;

		SDL_Surface * screen_surface = nullptr;

		void SetFocus(bool gained_focus)
		{
			has_focus = gained_focus;
			SDL_ShowCursor(! has_focus);
		}
	}
}

bool app::Init(Vector2i const & resolution, bool full_screen)
{
	// Initialize SDL.
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		std::cout << "Failed to initialize SDL: " << SDL_GetError();
		return false;
	}

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	//SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );

	// Get existing video info.
	/*const SDL_VideoInfo* video_info = SDL_GetVideoInfo();
	if (video_info == nullptr)
	{
		std::cout << "Failed to get video info: " << SDL_GetError();
		return false;
	}*/

	// Do the ... uh, thing.
	//int bpp = video_info->vfmt->BitsPerPixel;
	int flags = SDL_OPENGL | SDL_DOUBLEBUF | SDL_HWSURFACE;	// TODO: Are these optimal?

	if (full_screen)
	{
		//resolution.x = video_info->vfmt->width;
		//resolution.y = video_info->vfmt->height;
		flags |= SDL_FULLSCREEN;
	}

	screen_surface = SDL_SetVideoMode(resolution.x, resolution.y, 0, flags);
	if (screen_surface == nullptr)
	{
		std::cout << "Failed to set video mode: " << SDL_GetError();
		return false;
	}

	if (! InitGl())
	{
		return false;
	}

	SetFocus(true);

	ZeroObject(key_down);
	ZeroObject(button_down);

	mouse_position.x = resolution.y >> 1;
	mouse_position.y = resolution.y >> 1;
	window_size = resolution;

	return true;
}

bool app::IsKeyDown(KeyCode key_code)
{
	return key_down[key_code];
}

bool app::IsButtonDown(MouseButton mouse_button)
{
	return button_down[mouse_button];
}

SDL_Surface & app::GetVideoSurface()
{
	Assert(screen_surface);
	return * screen_surface;
}

Vector2i app::GetMousePosition()
{
	return mouse_position;
}

void app::SetMousePosition(Vector2i const & position)
{
	mouse_position = position;
	SDL_WarpMouse(mouse_position.x, mouse_position.y);
}

Vector2i app::GetWindowSize()
{
	return window_size;
}

bool app::GetEvent(Event & event)
{
	if (SDL_PollEvent(&event) > 0)
	{
		switch (event.type)
		{
			case SDL_VIDEORESIZE:
				window_size.x = event.resize.w;
				window_size.y = event.resize.h;
				return true;

			case SDL_ACTIVEEVENT:
				SetFocus(event.active.gain);
				break;

			case SDL_KEYDOWN:
				key_down [event.key.keysym.sym] = true;
				return true;

			case SDL_KEYUP:
				key_down [event.key.keysym.sym] = false;
				return true;

			case SDL_MOUSEBUTTONDOWN:
				button_down [event.button.button] = true;
				return true;

			case SDL_MOUSEBUTTONUP:
				button_down [event.button.button] = false;
				return true;

			case SDL_MOUSEMOTION:
				mouse_position.x = event.motion.x;
				mouse_position.y = event.motion.y;
				return true;

			default:
				return true;
		}
	}

	return false;
}

bool app::HasFocus()
{
	return has_focus;
}

app::TimeType app::GetTime()
{
	return SDL_GetTicks();
}

double app::TimeTypeToSeconds(TimeType t)
{
	return 0.001 * t;
}

app::TimeType app::SecondsToTimeType(float s)
{
	return s * 1000.f;
}
