//
//  Image.cpp
//  crag
//
//  Created by John on 12/3/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Image.h"

#include "Texture.h"

#include "core/app.h"

#include <SDL_image.h>

using namespace gfx;

namespace 
{
	auto opengl_rgba8_format = SDL_PIXELFORMAT_ARGB8888;
}

////////////////////////////////////////////////////////////////////////////////
// gfx::Image

Image::Image()
: _surface(nullptr)
{
}

Image::Image(Image && rhs)
: _surface(rhs._surface)
{
	rhs._surface = nullptr;
}

Image::~Image()
{
	Destroy();
}

Image & Image::operator=(Image && rhs)
{
	std::swap(_surface, rhs._surface);
	return * this;
}

bool Image::Create(geom::Vector2i const & size)
{
    Uint32 rmask, gmask, bmask, amask;

    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */
	static_assert(SDL_BYTEORDER == SDL_LIL_ENDIAN, "Probably needs some fixing");
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;

    _surface = SDL_CreateRGBSurface(0, size.x, size.y, 32, rmask, gmask, bmask, amask);
    if (! _surface)
    {
		CRAG_REPORT_SDL_ERROR();
		return false;
    }
	
	return true;
}

bool Image::Convert(Image const & source)
{
	ASSERT(source._surface != _surface);
	
	_surface = SDL_ConvertSurfaceFormat(source._surface, opengl_rgba8_format, 0);
	if (! _surface)
	{
		DEBUG_BREAK("Failed to convert image: %s", IMG_GetError());
		return false;
	}
	
	return true;
}

void Image::Destroy()
{
	if (_surface != nullptr)
	{
		SDL_FreeSurface(_surface);
	}
}

geom::Vector2i Image::GetSize() const
{
	return _surface 
	? geom::Vector2i(_surface->w, _surface->h) 
	: geom::Vector2i::Zero();
}

int Image::GetHeight() const
{
	return _surface ? _surface->h : 0;
}

int Image::GetWidth() const
{
	return _surface ? _surface->w : 0;
}

void Image::SetPixel(geom::Vector2i const & pos, Color4b const & color)
{
	ASSERT(pos.x >= 0 && pos.x < _surface->w);
	ASSERT(pos.y >= 0 && pos.x < _surface->h);

	char * line = reinterpret_cast<char *>(_surface->pixels) + pos.y * _surface->pitch;
	Uint32 * pixel = reinterpret_cast<Uint32 *>(line) + pos.x;

	Uint32 pixel_value = SDL_MapRGBA(_surface->format, color.r, color.g, color.b, color.a);
	* pixel = pixel_value;
}

void Image::Clear(Color4b const & color)
{
	if (_surface != nullptr)
	{
		SDL_Rect rect =
		{
			0,
			0,
			GetWidth(),
			GetHeight()
		};

		Uint32 rgba = SDL_MapRGBA(_surface->format, color.r, color.g, color.b, color.a);
		SDL_FillRect(_surface, & rect, rgba);
	}
}

void Image::Load(char const * filename)
{
	_surface = IMG_Load(app::GetAssetPath(filename));
	if (! _surface)
	{
		DEBUG_BREAK("Failed to load image; filename:%s; error:%s", filename, IMG_GetError());
	}

	if (_surface->format->format != opengl_rgba8_format)
	{
		DEBUG_MESSAGE("Incorrect image format; filename:%s", filename);
		auto raw_image = std::move(* this);
		Convert(raw_image);
	}
}

bool Image::Save(char const * filename)
{
	if (SDL_SaveBMP(_surface, filename) != 0)
	{
		CRAG_REPORT_SDL_ERROR();
		return false;
	}
	
	return true;
}

bool Image::CaptureScreen()
{
	geom::Vector2i window_size = app::GetResolution();
	
	if (! Create(window_size))
	{
		return false;
	}
	
	GL_CALL(glReadPixels(0, 0,
						   window_size.x, window_size.y, 
						   GL_RGBA, GL_UNSIGNED_BYTE,
						   _surface->pixels));
	
	return true;
}

bool Image::CopyVFlip(Image & dst, Image const & src)
{
	ASSERT(dst._surface != src._surface);
	
	geom::Vector2i size = src.GetSize();
	if (! dst.Create(size))
	{
		return false;
	}
	
	SDL_Rect srcrect = { 0, 0, size.x, 1 };
	SDL_Rect dstrect = { 0, size.y - 1, size.x, 1 };
	while (dstrect.y >= 0)
	{
		if (SDL_BlitSurface(src._surface, & srcrect, dst._surface, & dstrect) != 0)
		{
			ASSERT(false);
		}
		
		++ srcrect.y;
		-- dstrect.y;
	}
	
	return true;
}
