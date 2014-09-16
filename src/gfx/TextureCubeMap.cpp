//
//  TextureCubeMap.cpp
//  crag
//
//  Created by John on 2014-08-30.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "TextureCubeMap.h"

#include "Image.h"

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// gfx::TextureCubeMap member definitions

TextureCubeMap::TextureCubeMap(TextureCubeMap && rhs)
: Texture(std::move(rhs))
{
}

TextureCubeMap::TextureCubeMap(CubeMap<Image> const & images)
{
	Set(images);
}

TextureCubeMap & TextureCubeMap::operator=(TextureCubeMap && rhs)
{
	std::swap(_name, rhs._name);
	return * this;
}

void TextureCubeMap::Set(CubeMap<Image> const & images)
{
	constexpr auto layers = CubeMap<GLenum> ({{
		{{ GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X }},
		{{ GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y }},
		{{ GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z }}
	}});
	
	// otherwise, this need to be ready
	if (! IsInitialized())
	{
		Init();
	}
	
	Bind();

	ForEachSide([&] (int side, int pole) {
		auto & image = images[side][pole];

		if (! image.IsInitialized())
		{
			DEBUG_BREAK("Uninitialized CubMap Image [%d,%d]", side, pole);

			return;
		}
		
		auto & surface = * image._surface;

		auto width = surface.w;
		auto height = surface.h;
		auto pixels = surface.pixels;
		
		glTexImage2D(
			layers[side][pole], 0, GL_RGBA,
			width, height, 0, 
			GL_RGBA, GL_UNSIGNED_BYTE, pixels);	
	});

	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	Unbind();
}
