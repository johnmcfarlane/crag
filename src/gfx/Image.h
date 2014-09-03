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

#include "core/counted_object.h"

struct SDL_Surface;

namespace gfx 
{
	// Basic bitmap class / wrapper for SDL surface;
	// concerned only with OpenGL-friendly 32-bit true-color format
	class Image : private crag::counted_object<Image>
	{
		friend class Texture2d;
		friend class TextureCubeMap;
		
		OBJECT_NO_COPY(Image);
	public:

		Image();
		Image(Image && image);
		~Image();

		Image & operator=(Image && rhs);

		bool IsInitialized() const { return _surface != nullptr; }

		// init
		bool Create(geom::Vector2i const & size);
		bool Convert(Image const & source);
		void Destroy();

		// size info
		geom::Vector2i GetSize() const;
		int GetWidth() const;
		int GetHeight() const;

		// pixel access
		Color4b GetPixel(geom::Vector2i const & pos) const;
		void SetPixel(geom::Vector2i const & pos, Color4b const & color);

		// misc
		void Clear(Color4b const & color);
		void Load(char const * filename);
		bool Save(char const * filename);
		bool CaptureScreen();
		
		static bool CopyVFlip(Image & dst, Image const & src);

	private:

		SDL_Surface * _surface;
	};
}
