//
//  Texture2d.h
//  crag
//
//  Created by John McFarlane on 2014-08-30.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Texture.h"

namespace gfx
{
	// forward-declaration
	class Image;

	////////////////////////////////////////////////////////////////////////////////
	// OpenGl texture object wrapper
	
	class Texture2d : public Texture<GL_TEXTURE_2D>
	{
	public:
		Texture2d() = default;
		Texture2d(Texture2d && rhs) = default;
		Texture2d(Image const & image);
		
		Texture2d & operator=(Texture2d && rhs);
		
		void Set(Image const & image);
	};
}
