/*
 *  gfx/Image.h
 *  Crag
 *
 *  Created by John on 12/3/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Color.h"

#include "geom/Vector2.h"


struct SDL_PixelFormat;
struct SDL_Surface;


namespace gl
{
	class TextureRgba8;
}


namespace gfx 
{
	class Image
	{
	public:

		Image();
		~Image();

		operator bool () const { return surface != nullptr; }

		// formats
		typedef SDL_PixelFormat Format;
		static Format const & GetOpenGlRgba8Format();

		// init
		bool Create(Vector2i const & size, Format const & format = GetOpenGlRgba8Format());
		bool Convert(Image const & source, Format const & format = GetOpenGlRgba8Format());
		void Destroy();

		// size info
		Vector2i GetSize() const { return Vector2i(GetWidth(), GetHeight()); }
		int GetWidth() const;
		int GetHeight() const;

		// pixel access
		Color4b GetPixel(Vector2i const & pos) const;
		void SetPixel(Vector2i const & pos, Color4b const & color);
		//void const * GetPixelsPtr() const;	// use with caution
		bool CreateTexture(gl::TextureRgba8 & texture) const;

		// misc
		void Clear(Color4b const & color);
		void Load(char const * filename);
		bool Reformat(SDL_PixelFormat const & desired_format);
		bool FormatForScreen();
		bool FormatForOpenGl();

		friend bool Blit(Image * dst, Vector2i const & dst_pos, Image const & src, Vector2i const & src_pos, Vector2i const & size);

	private:

		SDL_Surface * surface;
	};

	bool Blit(Image * dst, Vector2i const & dst_pos, Image const & src, Vector2i const & src_pos, Vector2i const & size);
}


