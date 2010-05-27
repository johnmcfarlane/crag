/*
 *  gigl.h
 *  Crag
 *
 *  Created by John on 2/11/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "gl.h"
#include "debug.h"


namespace gl 
{
	// LoadMatrix
	inline void LoadMatrix(GLfloat const * m)
	{
		GLPP_CALL(glLoadMatrixf(m));
	}
	inline void LoadMatrix(GLdouble const * m)
	{
		GLPP_CALL(glLoadMatrixd(m));
	}

	// TexGenv
	inline void TexGenv(GLenum coord, GLenum pname, const GLfloat *params)
	{
		GLPP_CALL(glTexGenfv(coord, pname, params));
	}
	inline void TexGenv(GLenum coord, GLenum pname, const GLdouble *params)
	{
		GLPP_CALL(glTexGendv(coord, pname, params));
	}

	// Vertex3
	inline void Vertex3(GLfloat x, GLfloat y, GLfloat z)
	{
		GLPP_CALL(glVertex3f(x, y, z));
	}
	inline void Vertex3(GLdouble x, GLdouble y, GLdouble z)
	{
		GLPP_CALL(glVertex3d(x, y, z));
	}

	// Enable/Disable - checks that the call was necessary. 
	inline bool IsEnabled(GLenum cap)
	{
		bool enabled = glIsEnabled(cap);
		GLPP_VERIFY;
		return enabled;
	}
	
	inline void CheckEnabled(GLenum cap)
	{
		assert(IsEnabled(cap));
	}
	
	inline void CheckDisabled(GLenum cap)
	{
		assert(! IsEnabled(cap));
	}
	
	// Enable/Disable - checks that the call was necessary. 
	inline void Enable(GLenum cap, bool enabled = true)
	{
		assert(enabled != IsEnabled(cap));
		(enabled ? glEnable : glDisable)(cap);
		GLPP_VERIFY;
	}
	
	inline void Disable(GLenum cap)
	{
		Enable(cap, false);
	}
	
	// Return the name that is currently bound to the given target.
	inline GLuint GetBound(GLenum target_binding) 
	{
		GLint id;
		GLPP_CALL(glGetIntegerv(target_binding, & id));
		return id;
	}
	
	// Lots of get and set...
	template<GLenum PNAME> int GetInt()
	{
		GLint result;
		GLPP_CALL(glGetIntegerv(PNAME, & result));
		return result;
	}
	
	inline void DepthFunc(GLenum func) { GLPP_CALL(glDepthFunc(func)); }
	inline GLenum DepthFunc() { return GetInt<GL_DEPTH_FUNC>(); }
}
