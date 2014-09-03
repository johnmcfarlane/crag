//
//  Texture2d.cpp
//  crag
//
//  Created by John McFarlane on 2014-08-30.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Image.h"
#include "Texture2d.h"

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// gfx::Texture2d member definitions

Texture2d::Texture2d(Image const & image)
{
	Set(image);
}

void Texture2d::Set(Image const & image)
{
	auto surface = image._surface;

	// if image is uninitialized
	if (! surface)
	{
		// just need to deinitialize this
		if (IsInitialized())
		{
			Deinit();
		}

		return;
	}
	
	// otherwise, this need to be ready
	if (! IsInitialized())
	{
		Init();
	}
	
	auto width = surface->w;
	auto height = surface->h;
	auto pixels = surface->pixels;

	Bind();
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
						 width, height, 0, 
						 GL_RGBA, GL_UNSIGNED_BYTE, pixels));	
	GL_CALL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));
	Unbind();
}
