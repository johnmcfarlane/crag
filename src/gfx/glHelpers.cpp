//
//  glHelpers.cpp
//  crag
//
//  Created by John McFarlane on 2012-01-23.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "glHelpers.h"

#include "FrameBuffer.h"
#include "Texture.h"

////////////////////////////////////////////////////////////////////////////////
// Binding

#if ! defined(NDEBUG)
namespace gfx
{
#if defined(__ANDROID__)
	char const * ErrorString(GLenum)
	{
		return "GL error";
	}
#else
	char const * ErrorString(GLenum error)
	{
		return reinterpret_cast<char const *>(gluErrorString(error));
	}
#endif	// defined(__ANDROID__)
}
#endif	// ! defined(NDEBUG)

////////////////////////////////////////////////////////////////////////////////
// Binding

namespace gfx
{
	template <> GLuint GetBinding<GL_ARRAY_BUFFER>() 
	{ 
		return GetInt<GL_ARRAY_BUFFER_BINDING>(); 
	}

	template <> GLuint GetBinding<GL_ELEMENT_ARRAY_BUFFER>() 
	{ 
		return GetInt<GL_ELEMENT_ARRAY_BUFFER_BINDING>(); 
	}

	template <> GLuint GetBinding<GL_FRAMEBUFFER>() 
	{ 
		return GetInt<GL_FRAMEBUFFER_BINDING>(); 
	}

	template <> GLuint GetBinding<GL_RENDERBUFFER>() 
	{ 
		return GetInt<GL_RENDERBUFFER_BINDING>(); 
	}

	template <> GLuint GetBinding<GL_TEXTURE_2D>() 
	{ 
		return GetInt<GL_TEXTURE_BINDING_2D>(); 
	}
}


////////////////////////////////////////////////////////////////////////////////
// Misc

void gfx::Attach(FrameBuffer const & frame_buffer, Texture const & texture)
{
	if (! frame_buffer.IsBound())
	{
		DEBUG_BREAK("frame buffer is not bound");
	}
	
	GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture._name, 0));
}

