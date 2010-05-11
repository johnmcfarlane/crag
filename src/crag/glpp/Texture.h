/*
 *  Texture.h
 *  Crag
 *
 *  Created by John on 12/2/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "Name.h"


namespace gl
{
		
	template <GLenum INTERNAL_FORMAT> class Texture : public Name<GL_TEXTURE_2D>
	{
	public:
		inline void Resize(GLsizei width, GLsizei height);
		
		void Set(GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLvoid const * pixels);
		
		// nullptr passes back to regular back buffer
		template <GLenum IF, GLenum ATTACHMENT> friend void SetAsTarget(Texture<IF> * texture);
	};


	// Name specializations for GL_TEXTURE_2D

	template<> inline GLuint Init<GL_TEXTURE_2D>()
	{
		GLuint id;
		GLPP_CALL(glGenTextures(1, & id));
		return id;
	}

	template<> inline void Deinit<GL_TEXTURE_2D>(GLuint id)
	{
		GLPP_CALL(glDeleteTextures(1, & id));
	}

	template<> inline void Bind<GL_TEXTURE_2D>(GLuint id)
	{
		GLPP_CALL(glBindTexture(GL_TEXTURE_2D, id));
	}

	template<> inline GLuint GetBindingEnum<GL_TEXTURE_2D>()
	{
		return GL_TEXTURE_BINDING_2D;
	}

	template<GLenum INTERNAL_FORMAT, GLenum ATTACHMENT> void SetAsTarget(Texture<INTERNAL_FORMAT> * texture)
	{
		// Detach any existing texture.
		GLPP_CALL(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, ATTACHMENT, GL_TEXTURE_2D, 0, 0));
		
		if (texture != nullptr) {
			assert(texture->IsBound());
			GLPP_CALL(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, ATTACHMENT, GL_TEXTURE_2D, texture->id, 0));
		}
	}


	// TextureRgba8 - 'True color' specializations of Texture

	class TextureRgba8 : public Texture<GL_RGBA8>
	{
	public:
	};

	template<> inline void Texture<GL_RGBA8>::Resize(GLsizei width, GLsizei height)
	{
		assert(IsBound());
		GLPP_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 
							 width, height, 0, 
							 GL_RGBA, GL_UNSIGNED_BYTE, nullptr));	
	}

	template<> inline void Texture<GL_RGBA8>::Set(GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLvoid const * pixels)
	{
		assert(IsBound());
		GLPP_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 
								xoffset, yoffset, 
								width, height,
								GL_RGBA, GL_UNSIGNED_BYTE, pixels));
	}

	inline void SetAsTarget(Texture<GL_RGBA8> * texture)
	{
		SetAsTarget<GL_RGBA8, GL_COLOR_ATTACHMENT0_EXT>(texture);
	}


	// TextureDepth24 - 24-bit depth buffer specializations of Texture
	// TODO: Maybe change this to GL_DEPTH_COMPONENT a la 'Paul's Projects' tutorial

	class TextureDepth : public Texture<GL_DEPTH_COMPONENT>
	{
	};

	template<> inline void Texture<GL_DEPTH_COMPONENT>::Resize(GLsizei width, GLsizei height)
	{
		assert(IsBound());
	/*	GLPP_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
							 width, height, 0, 
							 GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr));*/
		GLPP_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 
							   width, height, 0, 
							   GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr));	
	}

	template<> inline void Texture<GL_DEPTH_COMPONENT>::Set(GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLvoid const * pixels)
	{
		// Seems unlikely that you'd ever want to call this.
		assert(IsBound());
		GLPP_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 
								xoffset, yoffset, 
								width, height,
								GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, pixels));
	}

	inline void SetAsTarget(Texture<GL_DEPTH_COMPONENT> * texture)
	{
		SetAsTarget<GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT_EXT>(texture);
	}

}
