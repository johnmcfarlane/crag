//
//  Image.h
//  crag
//
//  Created by John on 12/3/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Color.h"


struct SDL_PixelFormat;
struct SDL_Surface;


namespace gfx 
{
	// forward-declaration
	class Texture;

	// Basic bitmap class / wrapper for SDL surface.
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
		bool Create(geom::Vector2i const & size, Format const & format = GetOpenGlRgba8Format());
		bool Convert(Image const & source, Format const & format = GetOpenGlRgba8Format());
		void Destroy();

		// size info
		geom::Vector2i GetSize() const { return geom::Vector2i(GetWidth(), GetHeight()); }
		int GetWidth() const;
		int GetHeight() const;

		// pixel access
		Color4b GetPixel(geom::Vector2i const & pos) const;
		void SetPixel(geom::Vector2i const & pos, Color4b const & color);
		bool CreateTexture(Texture & texture) const;

		// misc
		void Clear(Color4b const & color);
		void Load(char const * filename);
		bool Save(char const * filename);
		bool CaptureScreen();
		bool Reformat(SDL_PixelFormat const & desired_format);
		bool FormatForOpenGl();

		static bool CopyVFlip(Image & dst, Image const & src);

	private:

		SDL_Surface * surface;
	};

	bool Blit(Image * dst, geom::Vector2i const & dst_pos, Image const & src, geom::Vector2i const & src_pos, geom::Vector2i const & size);
}


