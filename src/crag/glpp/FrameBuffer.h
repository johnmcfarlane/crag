/*
 *  Fbo.h
 *  Crag
 *
 *  Created by John on 12/12/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Name.h"

#if defined(__APPLE__)
#define glppBindFramebuffer glBindFramebuffer
#define glppDeleteFramebuffers glDeleteFramebuffers
#define glppGenFramebuffers glGenFramebuffers
#define glppFramebufferRenderbuffer glFramebufferRenderbuffer
#define GLPP_FRAMEBUFFER GL_FRAMEBUFFER
#define GLPP_FRAMEBUFFER_BINDING GL_FRAMEBUFFER_BINDING
#else
#define glppBindFramebuffer glBindFramebufferEXT
#define glppDeleteFramebuffers glDeleteFramebuffersEXT
#define glppGenFramebuffers glGenFramebuffersEXT
#define glppFramebufferRenderbuffer glFramebufferRenderbufferEXP
#define GLPP_FRAMEBUFFER GL_FRAMEBUFFER_EXT
#define GLPP_FRAMEBUFFER_BINDING GL_FRAMEBUFFER_BINDING_EXT
#endif


namespace gl
{
		
	// Frame Buffer Object
	class FrameBuffer : public Name<GLPP_FRAMEBUFFER>
	{
	public:
	};


	template<> inline GLuint Init<GLPP_FRAMEBUFFER>()
	{
		GLuint id;
		GLPP_CALL(glppGenFramebuffers(1, & id));
		return id;
	}

	template<> inline void Deinit<GLPP_FRAMEBUFFER>(GLuint id)
	{
		assert(id != 0);
		GLPP_CALL(glppDeleteFramebuffers(1, & id));
	}

	template<> inline void Bind<GLPP_FRAMEBUFFER>(GLuint id)
	{
		GLPP_CALL(glppBindFramebuffer(GLPP_FRAMEBUFFER, id));
	}

	template<> inline GLuint GetBindingEnum<GLPP_FRAMEBUFFER>()
	{
		return GLPP_FRAMEBUFFER_BINDING;
	}

}
