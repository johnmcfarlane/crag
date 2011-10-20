/*
 * 	gfx/Image.cpp
 *  Crag
 *
 *  Created by John on 12/3/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Image.h"

#include "sys/App.h"

#include "glpp/Texture.h"


namespace 
{
	gfx::Image::Format opengl_rgba8_format = 
	{
		SDL_PIXELFORMAT_RGBA8888,	// Uint32 format;
		nullptr,	// SDL_Palette *palette;
		32,			// Uint8  BitsPerPixel;
		4,			// Uint8  BytesPerPixel;
		{ 0, 0 },
		0x00ff0000,	// Uint32 Rmask;
		0x0000ff00,	// Uint32 Gmask;
		0x000000ff,	// Uint32 Bmask;
		0xff000000,	// Uint32 Amask;
		0,			// Uint8  Rloss;
		0,			// Uint8  Gloss;
		0,			// Uint8  Bloss;
		0,			// Uint8  Aloss;
		16,			// Uint8  Rshift;
		8,			// Uint8  Gshift;
		0,			// Uint8  Bshift;
		24,			// Uint8  Ashift;
		0,			// int refcount;
		nullptr,	// SDL_PixelFormat *next;
	};
}

static bool operator == (SDL_Palette const & lhs, SDL_Palette const & rhs)
{
	return lhs.ncolors == rhs.ncolors
		&& memcmp(lhs.colors, rhs.colors, sizeof(SDL_Color) * lhs.ncolors);
}

//static bool operator != (SDL_Palette const & lhs, SDL_Palette const & rhs)
//{
//	return ! (lhs == rhs);
//}

static bool operator == (SDL_PixelFormat const & lhs, SDL_PixelFormat const & rhs)
{
	return (lhs.palette == rhs.palette || * lhs.palette == * rhs.palette)
		&& lhs.BitsPerPixel == rhs.BitsPerPixel
		&& lhs.BytesPerPixel == rhs.BytesPerPixel
		&& lhs.Rloss == rhs.Rloss
		&& lhs.Gloss == rhs.Gloss
		&& lhs.Bloss == rhs.Bloss
		&& lhs.Aloss == rhs.Aloss
		&& lhs.Rshift == rhs.Rshift
		&& lhs.Gshift == rhs.Gshift
		&& lhs.Bshift == rhs.Bshift
		&& lhs.Ashift == rhs.Ashift
		&& lhs.Rmask == rhs.Rmask
		&& lhs.Gmask == rhs.Gmask
		&& lhs.Bmask == rhs.Bmask
		&& lhs.Amask == rhs.Amask;
}

static bool operator != (SDL_PixelFormat const & lhs, SDL_PixelFormat const & rhs)
{
	return ! (lhs == rhs);
}


////////////////////////////////////////////////////////////////////////////////
// gfx::Image

gfx::Image::Image()
: surface(nullptr)
{
}

gfx::Image::~Image()
{
	Destroy();
}

gfx::Image::Format const & gfx::Image::GetOpenGlRgba8Format()
{
	return opengl_rgba8_format;
}

bool gfx::Image::Create(Vector2i const & size, Format const & format)
{
	surface = SDL_CreateRGBSurface(0, 
								   size.x, size.y, 
								   format.BitsPerPixel, 
								   format.Rmask,
								   format.Gmask,
								   format.Bmask,
								   format.Amask);
	
	return surface != nullptr;
}

bool gfx::Image::Convert(Image const & source, Format const & format)
{
	surface = SDL_ConvertSurface(source.surface, & const_cast<SDL_PixelFormat &>(format), 0);
	return surface != nullptr;
}

void gfx::Image::Destroy()
{
	if (surface != nullptr)
	{
		SDL_FreeSurface(surface);
	}
}

int gfx::Image::GetWidth() const
{
	return surface ? surface->w : 0;
}

int gfx::Image::GetHeight() const
{
	return surface ? surface->h : 0;
}

void gfx::Image::SetPixel(Vector2i const & pos, Color4b const & color)
{
	Assert(pos.x >= 0 && pos.x < surface->w);
	Assert(pos.y >= 0 && pos.x < surface->h);

	char * line = reinterpret_cast<char *>(surface->pixels) + pos.y * surface->pitch;
	Uint32 * pixel = reinterpret_cast<Uint32 *>(line) + pos.x;

	Uint32 pixel_value = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a);
	* pixel = pixel_value;
}

bool gfx::Image::CreateTexture(gl::TextureRgba8 & texture) const
{
	if (surface == nullptr)
	{
		return false;
	}
	
	SDL_PixelFormat const & current_format = * surface->format;
	if (current_format != opengl_rgba8_format)
	{
		Image converted_image;
		converted_image.Convert(* this, opengl_rgba8_format);
		Assert (converted_image.surface == nullptr || (* converted_image.surface->format) == opengl_rgba8_format);
		return converted_image.CreateTexture(texture);
	}
	
	gl::GenTexture(texture);
	gl::BindTexture(texture);
	gl::TexImage(texture, GetWidth(), GetHeight(), surface->pixels);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	gl::UnbindTexture(texture);
	
	return true;
}


void gfx::Image::Clear(Color4b const & color)
{
	if (surface != nullptr)
	{
		SDL_Rect rect =
		{
			0,
			0,
			GetWidth(),
			GetHeight()
		};

		Uint32 rgba = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a);
		SDL_FillRect(surface, & rect, rgba);
	}
}

void gfx::Image::Load(char const * filename)
{
	surface = SDL_LoadBMP(filename);
}

bool gfx::Image::Save(char const * filename)
{
	if (SDL_SaveBMP(surface, filename) != 0)
	{
		sys::ReportSdlError("Failed to save bitmap");
		return false;
	}
	
	return true;
}

bool gfx::Image::CaptureScreen()
{
	Vector2i window_size = sys::GetWindowSize();
	Vector2i window_position = sys::GetWindowPosition();
	
	if (! Create(window_size))
	{
		return false;
	}
	
	GLPP_CALL(glReadPixels(0, 0,
						   window_size.x, window_size.y, 
						   GL_RGBA, GL_UNSIGNED_BYTE,
						   surface->pixels));
	
	return true;
}

bool gfx::Image::Reformat(SDL_PixelFormat const & desired_format)
{
	if (surface != nullptr)
	{
		SDL_PixelFormat const & current_format = * surface->format;
		if (current_format == desired_format)
		{
			return true;
		}
		
		SDL_Surface * new_surface = SDL_ConvertSurface(surface, & const_cast<SDL_PixelFormat &>(desired_format), 0);
		if (new_surface != nullptr)
		{
			SDL_FreeSurface(surface);
			surface = new_surface;
			return true;
		}
	}
	
	return false;
}

bool gfx::Image::FormatForOpenGl()
{
	if (surface != nullptr)
	{
		SDL_PixelFormat const & current_format = * surface->format;
		switch (current_format.BitsPerPixel)
		{
			case 32:
				return Reformat(opengl_rgba8_format);
				
			default:
				// not yet implemented
				Assert(false);
		}
	}
	
	return false;
}

bool gfx::Image::CopyVFlip(Image & dst, Image const & src)
{
	Vector2i size = src.GetSize();
	if (! dst.Create(size))
	{
		return false;
	}
	
	SDL_Rect srcrect = { 0, 0, size.x, 1 };
	SDL_Rect dstrect = { 0, size.y - 1, size.x, 1 };
	while (dstrect.y >= 0)
	{
		if (SDL_BlitSurface(src.surface, & srcrect, dst.surface, & dstrect) != 0)
		{
			Assert(false);
		}
		
		++ srcrect.y;
		-- dstrect.y;
	}
	
	return true;
}
