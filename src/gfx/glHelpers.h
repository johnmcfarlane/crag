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

#if defined(NDEBUG)
#define GL_VERIFY DO_NOTHING
#define GL_CALL(statement) statement;
#else
void VerifyGlCall(char const * file, int line, char const * statement = nullptr);
#define GL_VERIFY VerifyGlCall(__FILE__, __LINE__)
#define GL_CALL(statement) DO_STATEMENT(statement; VerifyGlCall(__FILE__, __LINE__, #statement);)
#endif


namespace gfx
{
	// Poll OpenGL for an int-type parameter value.
	template<GLenum PNAME> int GetInt()
	{
		GLint result;
		GL_CALL(glGetIntegerv(PNAME, & result));
		return result;
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Capability
	
	// Returns true if the given OpenGL parameter is true. 
	inline bool IsEnabled(GLenum cap)
	{
		bool enabled = (glIsEnabled(cap) == GL_TRUE);
		GL_VERIFY;
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
