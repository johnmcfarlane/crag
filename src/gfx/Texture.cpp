//
//  Texture.cpp
//  crag
//
//  Created by John McFarlane on 2012-01-23.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Texture.h"


using namespace gfx;

Texture::Texture()
: _name(0)
{
}

Texture::~Texture()
{
	ASSERT(! IsInitialized());
}

bool Texture::IsInitialized() const
{
	return _name != 0;
}

bool Texture::IsBound() const
{
	assert(IsInitialized());
	return GetBinding<GL_TEXTURE_2D>() == _name;
}

void Texture::Init() 
{ 
	assert(! IsInitialized());
	GL_CALL(glGenTextures(1, & _name)); 
}

void Texture::Deinit()
{
	assert(! IsBound());
	GL_CALL(glDeleteTextures(1, & _name)); 
	_name = 0;
}

void Texture::Bind() const
{
	assert(! IsBound());
	GL_CALL(glBindTexture(GL_TEXTURE_2D, _name)); 
}

void Texture::Unbind() const
{ 
	assert(IsBound());
#if ! defined(NDEBUG)
	GL_CALL(glBindTexture(GL_TEXTURE_2D, 0)); 
#endif
}

void Texture::SetImage(GLsizei width, GLsizei height, GLvoid const * pixels)
{
	assert(IsBound());
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
						 width, height, 0, 
						 GL_RGBA, GL_UNSIGNED_BYTE, pixels));	
}

void Texture::SetSubImage(GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLvoid const * pixels)
{
	assert(IsBound());
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 
							xoffset, yoffset, 
							width, height,
							GL_RGBA, GL_UNSIGNED_BYTE, pixels));
}
