/*
 *  Fbo.h
 *  Crag
 *
 *  Created by John on 12/12/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "Name.h"

/*#if 1
#define GLPP_FRAMEBUFFER GL_FRAMEBUFFER_EXT
#define giglGenFramebuffers glGenFramebuffersEXT
#define giglDeleteFramebuffers glDeleteFramebuffersEXT
#define giglBindFramebuffer glBindFramebufferEXT
#define GLPP_FRAMEBUFFER_BINDING GL_FRAMEBUFFER_BINDING_EXT
#else
#define GLPP_FRAMEBUFFER GL_FRAMEBUFFER_EXT
#define giglGenFramebuffers glGenFramebuffers
#define giglDeleteFramebuffers glDeleteFramebuffers
#define giglBindFramebuffer glBindFramebuffer
#define GLPP_FRAMEBUFFER_BINDING GL_FRAMEBUFFER_BINDING_EXT
#endif*/


namespace gl
{
		
	// Frame Buffer Object
	class FrameBuffer : public Name<GL_FRAMEBUFFER_EXT>
	{
	public:
	};


	template<> inline GLuint Init<GL_FRAMEBUFFER_EXT>()
	{
		GLuint id;
		GLPP_CALL(glGenFramebuffersEXT(1, & id));
		return id;
	}

	template<> inline void Deinit<GL_FRAMEBUFFER_EXT>(GLuint id)
	{
		assert(id != 0);
		GLPP_CALL(glDeleteFramebuffersEXT(1, & id));
	}

	template<> inline void Bind<GL_FRAMEBUFFER_EXT>(GLuint id)
	{
		GLPP_CALL(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id));
	}

	template<> inline GLuint GetBindingEnum<GL_FRAMEBUFFER_EXT>()
	{
		return GL_FRAMEBUFFER_BINDING_EXT;
	}

}
