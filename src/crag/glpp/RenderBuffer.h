/*
 *  RenderBuffer.h
 *  Crag
 *
 *  Created by John on 12/13/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "Name.h"

namespace gl 
{

	class RenderBuffer : public Name<GL_RENDERBUFFER_EXT>
	{
		typedef Name<GL_RENDERBUFFER_EXT> BaseClass;
		
	public:
		void ResizeForDepth(GLsizei width, GLsizei height)
		{
			assert(IsBound());
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
		}
		
		friend void AttachToFrameBuffer(RenderBuffer * render_buffer) 
		{
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, render_buffer->id);
		}
	};


	template<> inline GLuint Init<GL_RENDERBUFFER_EXT>()
	{
		GLuint id;
		GLPP_CALL(glGenRenderbuffersEXT(1, & id));
		return id;
	}
		   
	template<> inline void Deinit<GL_RENDERBUFFER_EXT>(GLuint id)
	{
		assert(id != 0);
		GLPP_CALL(glDeleteRenderbuffersEXT(1, & id));
	}
		   
	template<> inline void Bind<GL_RENDERBUFFER_EXT>(GLuint id)
	{
		GLPP_CALL(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, id));
	}
		   
	template<> inline GLuint GetBindingEnum<GL_RENDERBUFFER_EXT>()
	{
		return GL_RENDERBUFFER_BINDING_EXT;
	}
			   
}
