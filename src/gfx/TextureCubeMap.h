//
//  TextureCubeMap.h
//  crag
//
//  Created by John on 2014-08-30.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

#include "Texture.h"

namespace gfx
{
	class Image;
	
	////////////////////////////////////////////////////////////////////////////////
	// OpenGl CubeMap texture object wrapper
	
	class TextureCubeMap : public Texture<GL_TEXTURE_CUBE_MAP>
	{
	public:
		OBJECT_NO_COPY(TextureCubeMap);

		TextureCubeMap() = default;
		TextureCubeMap(TextureCubeMap && rhs);
		TextureCubeMap(CubeMap<Image> const & images);
		
		TextureCubeMap & operator=(TextureCubeMap && rhs);
		
		void Set(CubeMap<Image> const & images);
		
	private:
		GLuint _name = 0;
	};
}
