//
//  Texture.h
//  crag
//
//  Created by John on 12/2/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Name.h"


namespace gl
{
	
	////////////////////////////////////////////////////////////////////////////////
	// Texture function declarations
	
	template <GLenum FORMAT> class Texture;
	
	template<GLenum FORMAT> void GenTexture(Texture<FORMAT> & t);
	template<GLenum FORMAT> void DeleteTexture(Texture<FORMAT> & t);
	template<GLenum FORMAT> void BindTexture(Texture<FORMAT> const & t);
	template<GLenum FORMAT> void UnbindTexture(Texture<FORMAT> const & t);
	template<GLenum FORMAT> void TexImage(Texture<FORMAT> & t, GLsizei width, GLsizei height, GLvoid const * pixels = nullptr);
	template<GLenum FORMAT> void TexSubImage(Texture<FORMAT> & t, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLvoid const * pixels);


	////////////////////////////////////////////////////////////////////////////////
	// Texture classes
	
	template <GLenum FORMAT> class Texture : public Name<GL_TEXTURE_2D>
	{
	public:
		bool IsBound() const
		{
			assert(IsInitialized());
			return GetBinding<GL_TEXTURE_2D>() == id;
		}
		
		friend void GenTexture<FORMAT>(Texture & t);
		friend void DeleteTexture<FORMAT>(Texture & t);
		friend void BindTexture<FORMAT>(Texture const & t);
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Texture function declarations
	
	template<GLenum FORMAT> void GenTexture(Texture<FORMAT> & t) 
	{ 
		assert(! t.IsInitialized());
		GLPP_CALL(glGenTextures(1, & t.id)); 
	}
	
	template<GLenum FORMAT> void DeleteTexture(Texture<FORMAT> & t) 
	{ 
		assert(! t.IsBound());
		GLPP_CALL(glDeleteTextures(1, & t.id)); 
		t.id = 0;
	}
	
	template<GLenum FORMAT> void BindTexture(Texture<FORMAT> const & t) 
	{
		assert(! t.IsBound());
		GLPP_CALL(glBindTexture(GL_TEXTURE_2D, t.id)); 
	}
	
	template<GLenum FORMAT> void UnbindTexture(Texture<FORMAT> const & t) 
	{ 
		assert(t.IsBound());
#if ! defined(NDEBUG)
		GLPP_CALL(glBindTexture(GL_TEXTURE_2D, 0)); 
#endif
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// TextureRgba8 Texture specializations
	
	class TextureRgba8 : public Texture<GL_RGBA8>
	{
	};
	
	template<> inline void TexImage<GL_RGBA8>(Texture<GL_RGBA8> & t, GLsizei width, GLsizei height, GLvoid const * pixels)
	{
		assert(t.IsBound());
		GLPP_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 
							   width, height, 0, 
							   GL_RGBA, GL_UNSIGNED_BYTE, pixels));	
	}
	
	template<> inline void TexSubImage<GL_RGBA8>(Texture<GL_RGBA8> & t, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLvoid const * pixels)
	{
		assert(t.IsBound());
		GLPP_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 
								  xoffset, yoffset, 
								  width, height,
								  GL_RGBA, GL_UNSIGNED_BYTE, pixels));
	}

}
