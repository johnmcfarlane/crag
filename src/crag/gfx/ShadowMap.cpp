/*
 *  ShadowMap.cpp
 *  Crag
 *
 *  Created by John on 12/23/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#include "pch.h"

#include "ShadowMap.h"


// note: function should never be called for the same object with a different value of size
void gfx::ShadowMap::Begin(int size)
{
	// Get the texture.
	if (! depth_texture.IsInitialized()) {
		InitTextures(size);
	}
	
	Assert(depth_texture.IsInitialized());
	Bind(& depth_texture);
	SetAsTarget(&  depth_texture);

#if (SHADOW_MAP_TEST >= 2)
	Assert(color_texture.IsInitialized());
	Bind(& color_texture);
	SetAsTarget(&  color_texture);
#endif

	Assert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT);
	GLPP_VERIFY;
}

void gfx::ShadowMap::InitTextures(int size)
{
	depth_texture.Init();
	Bind(& depth_texture);
/*	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_ALPHA);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
*/	depth_texture.Resize(size, size);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
#if (SHADOW_MAP_TEST >= 2)
	color_texture.Init();
	Bind(& color_texture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	color_texture.Resize(size, size);
#endif
	
	GLPP_VERIFY;
}
