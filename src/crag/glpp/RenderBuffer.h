/*
 *  RenderBuffer.h
 *  Crag
 *
 *  Created by John on 12/13/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#if 0
#include "Name.h"

#include "FrameBuffer.h"


#if defined(__APPLE__) && 0
#define glppBindRenderbuffer		glBindRenderbuffer
#define glppDeleteRenderbuffers		glDeleteRenderbuffers
#define glppGenRenderbuffers		glGenRenderbuffers
#define glppRenderbufferStorage		glRenderbufferStorage
#define GLPP_DEPTH_ATTACHMENT		GL_DEPTH_ATTACHMENT
#define GLPP_RENDERBUFFER			GL_RENDERBUFFER
#define GLPP_RENDERBUFFER_BINDING	GL_RENDERBUFFER_BINDING
#else
#define glppBindRenderbuffer		glBindRenderbufferEXT
#define glppDeleteRenderbuffers 	glDeleteRenderbuffersEXT
#define glppGenRenderbuffers		glGenRenderbuffersEXT
#define glppRenderbufferStorage 	glRenderbufferStorageEXT
#define GLPP_DEPTH_ATTACHMENT		GL_DEPTH_ATTACHMENT_EXT
#define GLPP_RENDERBUFFER			GL_RENDERBUFFER_EXT
#define GLPP_RENDERBUFFER_BINDING	GL_RENDERBUFFER_BINDING_EXT
#endif


namespace gl 
{

	class RenderBuffer : public Name<GLPP_RENDERBUFFER>
	{
		typedef Name<GLPP_RENDERBUFFER> BaseClass;
		
	public:
		void ResizeForDepth(GLsizei width, GLsizei height)
		{
			assert(IsBound());
			glppRenderbufferStorage(GLPP_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		}
		
		friend void AttachToFrameBuffer(RenderBuffer * render_buffer) 
		{
			glppFramebufferRenderbuffer(GLPP_FRAMEBUFFER, GLPP_DEPTH_ATTACHMENT, GLPP_RENDERBUFFER, render_buffer->id);
		}
	};


	template<> inline GLuint Init<GLPP_RENDERBUFFER>()
	{
		GLuint id;
		GLPP_CALL(glppGenRenderbuffers(1, & id));
		return id;
	}
		   
	template<> inline void Deinit<GLPP_RENDERBUFFER>(GLuint id)
	{
		assert(id != 0);
		GLPP_CALL(glppDeleteRenderbuffers(1, & id));
	}
		   
	template<> inline void Bind<GLPP_RENDERBUFFER>(GLuint id)
	{
		GLPP_CALL(glppBindRenderbuffer(GLPP_RENDERBUFFER, id));
	}
		   
	template<> inline GLuint GetBindingEnum<GLPP_RENDERBUFFER>()
	{
		return GLPP_RENDERBUFFER_BINDING;
	}
			   
}
#endif
