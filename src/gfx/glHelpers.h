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
#define GL_VERIFY DO_STATEMENT(\
	GLenum error = glGetError(); \
	if (error != GL_NO_ERROR) { \
		DEBUG_BREAK("GL error %X (%s)", error, gluErrorString(error)); } )

#define GL_CALL(statement) DO_STATEMENT(\
	statement; \
	GLenum error = glGetError(); \
	if (error != GL_NO_ERROR) { \
		DEBUG_BREAK("GL error %X (%s): '%s'", error, gluErrorString(error), #statement); } )
#endif


namespace gfx
{
	// forward-declarations
	class FrameBuffer;
	class Texture;
	
	////////////////////////////////////////////////////////////////////////////////
	// State / Capability
	
	// Poll OpenGL for an int-type parameter value.
	template<GLenum PNAME> int GetInt()
	{
		GLint result;
		GL_CALL(glGetIntegerv(PNAME, & result));
		return result;
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
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Misc
	
	void Attach(FrameBuffer const & frame_buffer, Texture const & texture);
}
