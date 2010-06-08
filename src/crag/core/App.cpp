/*
 *  Input.cpp
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
#include "core/Mutex.h"

#include <SDL_image.h>


namespace ANONYMOUS
{
	bool has_focus = true;
	
	bool key_down [KEY_MAX];
	bool button_down [app::BUTTON_MAX];
	
	Vector2i mouse_position;
	Vector2i window_size;
	
	SDL_Surface * screen_surface = nullptr;
	
	void SetFocus(bool gained_focus)
	{
		has_focus = gained_focus;
		SDL_ShowCursor(! has_focus);
	}
}


bool app::Init(Vector2i resolution, bool full_screen, char const * title)
{
	// Initialize SDL.
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		std::cout << "Failed to initialize SDL: " << SDL_GetError();
		return false;
	}
	
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 1 );
	
	// Get existing video info.
	const SDL_VideoInfo* video_info = SDL_GetVideoInfo();
	if (video_info == nullptr)
	{
		std::cout << "Failed to get video info: " << SDL_GetError();
		return false;
	}
	
	// Do the ... uh, thing.
	int bpp = video_info->vfmt->BitsPerPixel;
	int flags = SDL_OPENGL | SDL_DOUBLEBUF | SDL_HWSURFACE;
	
	if (full_screen)
	{
		resolution.x = video_info->current_w;
		resolution.y = video_info->current_h	;
		flags |= SDL_FULLSCREEN;
	}
	
	screen_surface = SDL_SetVideoMode(resolution.x, resolution.y, bpp, flags);
	if (screen_surface == nullptr)
	{
		std::cout << "Failed to set video mode: " << SDL_GetError() << '\n';
		std::cout << resolution.x << 'x';
		std::cout << resolution.y << 'x';
		std::cout << bpp << " (" << std::hex;
		std::cout << flags << std::dec << ")\n";
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
	
	SDL_WM_SetCaption(title, nullptr);
	
	return true;
}

bool app::InitGl()
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

bool app::IsKeyDown(KeyCode key_code)
{
	return key_down[key_code];
}

bool app::IsButtonDown(MouseButton mouse_button)
{
	return button_down[mouse_button];
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

SDL_Surface & app::GetVideoSurface()
{
	Assert(screen_surface);
	return * screen_surface;
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
				SetFocus(event.active.gain != 0);
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
	return .001 * SDL_GetTicks();
}

void app::Sleep(TimeType t)
{
	SDL_Delay(static_cast<Uint32>(t * 1000));
	//boost::this_thread::sleep(boost::posix_time::milliseconds(t));
}

