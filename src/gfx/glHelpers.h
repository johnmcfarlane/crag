//
//  glHelpers.h
//  crag
//
//  Created by John on 2010-02-11.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

//////////////////////////////////////////////////////////////////////
// OpenGl error checking

#if defined(CRAG_RELEASE)
#define GL_VERIFY DO_NOTHING
#define GL_CALL(statement) statement;
#else
#define GL_VERIFY DO_STATEMENT(\
	GLenum error = glGetError(); \
	if (error != GL_NO_ERROR) { \
		DEBUG_BREAK("GL error 0x%X (%s)", error, gfx::ErrorString(error)); } )

#define GL_CALL(statement) DO_STATEMENT(\
	ASSERT(glGetError() == GL_NO_ERROR); \
	statement; \
	GLenum error = glGetError(); \
	if (error != GL_NO_ERROR) { \
		DEBUG_BREAK("GL error 0x%X (%s): '%s'", error, gfx::ErrorString(error), #statement); } )
#endif

namespace gfx
{
	////////////////////////////////////////////////////////////////////////////////
	// Debug output

#if defined(CRAG_DEBUG)
	char const * ErrorString(GLenum error);
#endif

	////////////////////////////////////////////////////////////////////////////////
	// State / Capability
	
	// Poll OpenGL for an int-type parameter value.
	template<GLenum PNAME> int GetInt()
	{
		GLint result;
		GL_CALL(glGetIntegerv(PNAME, & result));
		return result;
	}
	
	// Poll OpenGL for an int-type parameter value.
	template<GLenum PNAME> bool GetBool()
	{
		GLboolean result;
		GL_CALL(glGetBooleanv(PNAME, & result));
		return result != GL_FALSE;
	}
	
	// Returns true if the given OpenGL parameter is true. 
	inline bool IsEnabled(GLenum cap)
	{
		bool enabled;
		GL_CALL(enabled = (glIsEnabled(cap) != GL_FALSE));
		return enabled;
	}
	
	// Enable the given OpenGL parameter. Checks for redundnacy.
	inline void Enable(GLenum cap)
	{
		assert(! IsEnabled(cap));
		GL_CALL(glEnable(cap));
	}
	
	// Disable the given OpenGL parameter. Checks for redundnacy.
	inline void Disable(GLenum cap)
	{
		assert(IsEnabled(cap));
		GL_CALL(glDisable(cap));
	}
	

	////////////////////////////////////////////////////////////////////////////////
	// Binding

	template <GLenum TARGET> GLuint GetBinding();
}
