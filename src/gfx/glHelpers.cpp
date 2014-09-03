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

namespace gfx
{
	////////////////////////////////////////////////////////////////////////////////
	// Debugging

#if ! defined(NDEBUG)
#if defined(CRAG_USE_GLES)
	char const * ErrorString(GLenum)
	{
		return "GL error";
	}
#else
	char const * ErrorString(GLenum error)
	{
		return reinterpret_cast<char const *>(gluErrorString(error));
	}
#endif	// defined(CRAG_USE_GLES)
#endif	// ! defined(NDEBUG)

	////////////////////////////////////////////////////////////////////////////////
	// Binding

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

	template <> GLuint GetBinding<GL_TEXTURE_CUBE_MAP>() 
	{ 
		return GetInt<GL_TEXTURE_BINDING_CUBE_MAP>(); 
	}
}

