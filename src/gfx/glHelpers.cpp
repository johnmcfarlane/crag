//
//  glHelpers.cpp
//  crag
//
//  Created by John McFarlane on 2012-01-23.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "glHelpers.h"


////////////////////////////////////////////////////////////////////////////////
// OpenGl error checking function

#if ! defined(NDEBUG)

void VerifyGlCall(char const * file, int line, char const * statement)
{
	GLenum error = glGetError(); 
	if (error == GL_NO_ERROR) 
	{
		return;
	}
	
	std::cerr	<< file << ':' 
	<< std::dec << line 
	<< ": error 0x" << std::hex << error << ": " 
	<< gluErrorString(error); 
	
	if (statement != nullptr)
	{
		std::cerr << ", \"" << statement << '"';
	}
	
	std::cerr << std::endl;
	assert(false);
}

#endif

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
