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
	// Begin/End
	inline void Begin(GLenum mode)
	{
		GLPP_VERIFY;
		glBegin(mode);
	}
	inline void End()
	{
		glEnd();
		GLPP_VERIFY;
	}
	
	// LoadMatrix
	inline void LoadMatrix(GLfloat const * m)
	{
		GLPP_CALL(glLoadMatrixf(m));
	}
	inline void LoadMatrix(GLdouble const * m)
	{
		GLPP_CALL(glLoadMatrixd(m));
	}

	inline void Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
	{
		GLPP_CALL(glViewport(x, y, width, height));
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
		/*GLPP_CALL*/(glVertex3f(x, y, z));
	}
	inline void Vertex3(GLdouble x, GLdouble y, GLdouble z)
	{
		/*GLPP_CALL*/(glVertex3d(x, y, z));
	}

	// Enable/Disable - checks that the call was necessary. 
	inline bool IsEnabled(GLenum cap)
	{
		bool enabled = (glIsEnabled(cap) == GL_TRUE);
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
	
	// set/get depth function
	inline void SetDepthFunc(GLenum func) { GLPP_CALL(glDepthFunc(func)); }
	inline GLenum GetDepthFunc() { return GetInt<GL_DEPTH_FUNC>(); }
	
	// set current red, green and blue color and set alpha to 1 (full).
	template <typename COMPONENT> void SetColor(COMPONENT red, COMPONENT green, COMPONENT blue);
	template <> inline void SetColor<GLbyte>(GLbyte red, GLbyte green, GLbyte blue)			{ /*GLPP_CALL*/(glColor3b (red, green, blue)); }
	template <> inline void SetColor<GLdouble>(GLdouble red, GLdouble green, GLdouble blue)	{ /*GLPP_CALL*/(glColor3d (red, green, blue)); }
	template <> inline void SetColor<GLfloat>(GLfloat red, GLfloat green, GLfloat blue)		{ /*GLPP_CALL*/(glColor3f (red, green, blue)); }
	template <> inline void SetColor<GLint>(GLint red, GLint green, GLint blue)				{ /*GLPP_CALL*/(glColor3i (red, green, blue)); }
	template <> inline void SetColor<GLshort>(GLshort red, GLshort green, GLshort blue)		{ /*GLPP_CALL*/(glColor3s (red, green, blue)); }
	template <> inline void SetColor<GLubyte>(GLubyte red, GLubyte green, GLubyte blue)		{ /*GLPP_CALL*/(glColor3ub(red, green, blue)); }
	template <> inline void SetColor<GLuint>(GLuint red, GLuint green, GLuint blue)			{ /*GLPP_CALL*/(glColor3ui(red, green, blue)); }
	template <> inline void SetColor<GLushort>(GLushort red, GLushort green, GLushort blue)	{ /*GLPP_CALL*/(glColor3us(red, green, blue)); }
	
	template <typename COMPONENT> void SetColor(COMPONENT red, COMPONENT green, COMPONENT blue, COMPONENT alpha);
	template <> inline void SetColor(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)			{ /*GLPP_CALL*/(glColor4b (red, green, blue, alpha)); }
	template <> inline void SetColor(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)	{ /*GLPP_CALL*/(glColor4d (red, green, blue, alpha)); }
	template <> inline void SetColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)		{ /*GLPP_CALL*/(glColor4f (red, green, blue, alpha)); }
	template <> inline void SetColor(GLint red, GLint green, GLint blue, GLint alpha)				{ /*GLPP_CALL*/(glColor4i (red, green, blue, alpha)); }
	template <> inline void SetColor(GLshort red, GLshort green, GLshort blue, GLshort alpha)		{ /*GLPP_CALL*/(glColor4s (red, green, blue, alpha)); }
	template <> inline void SetColor(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)		{ /*GLPP_CALL*/(glColor4ub(red, green, blue, alpha)); }
	template <> inline void SetColor(GLuint red, GLuint green, GLuint blue, GLuint alpha)			{ /*GLPP_CALL*/(glColor4ui(red, green, blue, alpha)); }
	template <> inline void SetColor(GLushort red, GLushort green, GLushort blue, GLushort alpha)	{ /*GLPP_CALL*/(glColor4us(red, green, blue, alpha)); }
	
	template<typename COMPONENT> void SetColor(COMPONENT const * rgba)					{ SetColor(rgba[0], rgba[1], rgba[2], rgba[3]); }
	
	// get current color
	inline void GetColor(GLdouble rgba [4])	{ GLPP_CALL(glGetDoublev(GL_CURRENT_COLOR, rgba)); }
	inline void GetColor(GLfloat rgba [4])	{ GLPP_CALL(glGetFloatv(GL_CURRENT_COLOR, rgba)); }
	inline void GetColor(GLint rgba [4])	{ GLPP_CALL(glGetIntegerv(GL_CURRENT_COLOR, rgba)); }
}
