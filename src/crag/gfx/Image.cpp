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

#include "core/App.h"

#include "glpp/Texture.h"


namespace ANONYMOUS 
{
	gfx::Image::Format opengl_rgba8_format = 
	{
		nullptr,	// SDL_Palette *palette;
		32,			// Uint8  BitsPerPixel;
		4,			// Uint8  BytesPerPixel;
		0,			// Uint8  Rloss;
		0,			// Uint8  Gloss;
		0,			// Uint8  Bloss;
		0,			// Uint8  Aloss;
		0,			// Uint8  Rshift;
		8,			// Uint8  Gshift;
		16,			// Uint8  Bshift;
		24,			// Uint8  Ashift;
		0x000000ff,	// Uint32 Rmask;
		0x0000ff00,	// Uint32 Gmask;
		0x00ff0000,	// Uint32 Bmask;
		0xff000000,	// Uint32 Amask;
		0,			// Uint32 colorkey;
		255,		// Uint8  alpha;
	};

	//	#define RMASK 0xff000000
	//	#define GMASK 0x00ff0000
	//	#define BMASK 0x0000ff00
	//	#define AMASK 0x000000ff

	bool operator == (SDL_Palette const & lhs, SDL_Palette const & rhs)
	{
		return lhs.ncolors == rhs.ncolors
			&& memcmp(lhs.colors, rhs.colors, sizeof(SDL_Color) * lhs.ncolors);
	}

	bool operator != (SDL_Palette const & lhs, SDL_Palette const & rhs)
	{
		return ! (lhs == rhs);
	}

	bool operator == (SDL_PixelFormat const & lhs, SDL_PixelFormat const & rhs)
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

	bool operator != (SDL_PixelFormat const & lhs, SDL_PixelFormat const & rhs)
	{
		return ! (lhs == rhs);
	}

}	// anonymous namespace


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

gfx::Image::Format const & gfx::Image::GetVideoFormat()
{
	SDL_Surface const & video_surface = app::GetVideoSurface();
	SDL_PixelFormat const & video_format = ref(video_surface.format);
	return video_format;
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
	
	texture.Init();
	gl::Bind(& texture);
	texture.Resize(GetWidth(), GetHeight());
	texture.Set(0, 0, GetWidth(), GetHeight(), reinterpret_cast<GLvoid const *>(surface->pixels));
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	
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
	surface = IMG_Load(filename);
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

bool gfx::Image::FormatForScreen()
{
	SDL_PixelFormat const & video_format = * SDL_GetVideoSurface()->format;
	return Reformat(video_format);
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

bool gfx::Blit(Image * dst, Vector2i const & dst_pos, Image const & src, Vector2i const & src_pos, Vector2i const & size)
{
	SDL_Surface * dst_surface = (dst == nullptr)
			? SDL_GetVideoSurface()
			: dst->surface;

	SDL_Rect srcrect =
	{
		src_pos.x,
		src_pos.y,
		size.x,
		size.y,
	};

	SDL_Rect dstrect =
	{
		dst_pos.x,
		dst_pos.y,
		size.x,	// unnecessary?
		size.y,	// unnecessary?
	};

	return SDL_BlitSurface(src.surface, & srcrect, dst_surface, & dstrect) == 0;
}
