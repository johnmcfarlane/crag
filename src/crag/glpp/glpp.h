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
	
#if defined(WIN32)
#define GLPP_USE_ARB
#endif
	
#if defined(GLPP_USE_ARB)
#define GLPP_DEFINE_CONSTANT_ARB(NAME) NAME = GL_##NAME##_ARB
#define GLPP_CALL_FUNCTION_ARB(NAME, PARAMS) GLPP_CALL(gl##NAME##ARB PARAMS)
#else
#define GLPP_DEFINE_CONSTANT_ARB(NAME) NAME = GL_##NAME
#define GLPP_CALL_FUNCTION_ARB(NAME, PARAMS) GLPP_CALL(gl##NAME PARAMS)
#endif
	
	enum
	{
		GLPP_DEFINE_CONSTANT_ARB(ARRAY_BUFFER), 
		GLPP_DEFINE_CONSTANT_ARB(ARRAY_BUFFER_BINDING), 
		GLPP_DEFINE_CONSTANT_ARB(ELEMENT_ARRAY_BUFFER), 
		GLPP_DEFINE_CONSTANT_ARB(ELEMENT_ARRAY_BUFFER_BINDING)
	};
	
	enum BufferDataUsage
	{
		GLPP_DEFINE_CONSTANT_ARB(STREAM_DRAW),
		GLPP_DEFINE_CONSTANT_ARB(STATIC_DRAW), 
		GLPP_DEFINE_CONSTANT_ARB(DYNAMIC_DRAW), 
		GLPP_DEFINE_CONSTANT_ARB(STREAM_READ),
		GLPP_DEFINE_CONSTANT_ARB(STATIC_READ), 
		GLPP_DEFINE_CONSTANT_ARB(DYNAMIC_READ), 
		GLPP_DEFINE_CONSTANT_ARB(STREAM_COPY),
		GLPP_DEFINE_CONSTANT_ARB(STATIC_COPY), 
		GLPP_DEFINE_CONSTANT_ARB(DYNAMIC_COPY), 
	};
	
	////////////////////////////////////////////////////////////////////////////////
	// General state access
	
	// Lots of get and set...
	template<GLenum PNAME> bool GetBool()
	{
		GLboolean result;
		GLPP_CALL(glGetBooleanv(PNAME, & result));
		return result != GL_FALSE;
	}
	
	// Lots of get and set...
	template<GLenum PNAME> int GetInt()
	{
		GLint result;
		GLPP_CALL(glGetIntegerv(PNAME, & result));
		return result;
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Capability
	
	// Enable/Disable - checks that the call was necessary. 
	inline bool IsEnabled(GLenum cap)
	{
		bool enabled = (glIsEnabled(cap) == GL_TRUE);
		GLPP_VERIFY;
		return enabled;
	}
	
	// Enable/Disable - checks that the call was necessary. 
	inline void Enable(GLenum cap)
	{
		assert(! IsEnabled(cap));
		GLPP_CALL(glEnable(cap));
	}
	
	inline void Disable(GLenum cap)
	{
		assert(IsEnabled(cap));
		GLPP_CALL(glDisable(cap));
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Specific state access
	
	// set/get depth function
	inline void SetDepthFunc(GLenum func) 
	{ 
		GLPP_CALL(glDepthFunc(func)); 
	}
	
	inline GLenum GetDepthFunc() 
	{ 
		return GetInt<GL_DEPTH_FUNC>(); 
	}
	
	// set/get depth mask
	inline void SetDepthMask(bool flag) 
	{ 
		GLPP_CALL(glDepthMask(flag)); 
	}
	
	inline bool GetDepthMask() 
	{ 
		return GetBool<GL_DEPTH_WRITEMASK>();
	}
	
	// set/get shade model
	inline void ShadeModel(GLenum model)
	{
		GLPP_CALL(glShadeModel(model));
	}
	inline GLenum GetShadeModel()
	{
		return GetInt<GL_SHADE_MODEL>();
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Non-buffer poly rendering
	
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
	
	// Vertex3
	inline void Vertex3(GLfloat x, GLfloat y, GLfloat z)
	{
		glVertex3f(x, y, z);
	}
	inline void Vertex3(GLdouble x, GLdouble y, GLdouble z)
	{
		glVertex3d(x, y, z);
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Matrix
	
	inline void LoadMatrix(GLfloat const * m)
	{
		GLPP_CALL(glLoadMatrixf(m));
	}
	inline void LoadMatrix(GLdouble const * m)
	{
		GLPP_CALL(glLoadMatrixd(m));
	}
	
	inline void MatrixMode(GLenum mode)
	{
		GLPP_CALL(glMatrixMode(mode));
	}
	inline GLenum GetMatrixMode()
	{
		return GetInt<GL_MATRIX_MODE>();
	}
	
	inline void Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
	{
		GLPP_CALL(glViewport(x, y, width, height));
	}
	
	// Scale
	inline void Scale(GLfloat x, GLfloat y, GLfloat z)
	{
		GLPP_CALL(glScalef(x, y, z));
	}
	inline void Scale(GLdouble x, GLdouble y, GLdouble z)
	{
		GLPP_CALL(glScaled(x, y, z));
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Binding
	
	template <GLenum TARGET> GLuint GetBinding();
	template <> inline GLuint GetBinding<ARRAY_BUFFER>() { return GetInt<ARRAY_BUFFER_BINDING>(); }
	template <> inline GLuint GetBinding<ELEMENT_ARRAY_BUFFER>() { return GetInt<ELEMENT_ARRAY_BUFFER_BINDING>(); }
	template <> inline GLuint GetBinding<GL_RENDERBUFFER_EXT>() { return GetInt<GL_RENDERBUFFER_BINDING_EXT>(); }
	template <> inline GLuint GetBinding<GL_TEXTURE_2D>() { return GetInt<GL_TEXTURE_BINDING_2D>(); }
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Texture
	
	inline void TexGenv(GLenum coord, GLenum pname, const GLfloat *params)
	{
		GLPP_CALL(glTexGenfv(coord, pname, params));
	}
	inline void TexGenv(GLenum coord, GLenum pname, const GLdouble *params)
	{
		GLPP_CALL(glTexGendv(coord, pname, params));
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Color
	
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
	
	// set current red, green, blue and alpha.
	template <typename COMPONENT> void SetColor(COMPONENT red, COMPONENT green, COMPONENT blue, COMPONENT alpha);
	template <> inline void SetColor(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)			{ /*GLPP_CALL*/(glColor4b (red, green, blue, alpha)); }
	template <> inline void SetColor(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)	{ /*GLPP_CALL*/(glColor4d (red, green, blue, alpha)); }
	template <> inline void SetColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)		{ /*GLPP_CALL*/(glColor4f (red, green, blue, alpha)); }
	template <> inline void SetColor(GLint red, GLint green, GLint blue, GLint alpha)				{ /*GLPP_CALL*/(glColor4i (red, green, blue, alpha)); }
	template <> inline void SetColor(GLshort red, GLshort green, GLshort blue, GLshort alpha)		{ /*GLPP_CALL*/(glColor4s (red, green, blue, alpha)); }
	template <> inline void SetColor(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)		{ /*GLPP_CALL*/(glColor4ub(red, green, blue, alpha)); }
	template <> inline void SetColor(GLuint red, GLuint green, GLuint blue, GLuint alpha)			{ /*GLPP_CALL*/(glColor4ui(red, green, blue, alpha)); }
	template <> inline void SetColor(GLushort red, GLushort green, GLushort blue, GLushort alpha)	{ /*GLPP_CALL*/(glColor4us(red, green, blue, alpha)); }
	
	// set current red, green, blue and alpha from an array.
	template<typename COMPONENT> void SetColor(COMPONENT const * rgba)					{ SetColor(rgba[0], rgba[1], rgba[2], rgba[3]); }
	
	// get current color
	inline void GetColor(GLdouble rgba [4])	{ GLPP_CALL(glGetDoublev(GL_CURRENT_COLOR, rgba)); }
	inline void GetColor(GLfloat rgba [4])	{ GLPP_CALL(glGetFloatv(GL_CURRENT_COLOR, rgba)); }
	inline void GetColor(GLint rgba [4])	{ GLPP_CALL(glGetIntegerv(GL_CURRENT_COLOR, rgba)); }
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Misc functions
	
	inline void Finish()
	{
		glFinish();
		GLPP_VERIFY;
	}
	inline void Flush()
	{
		glFlush();
		GLPP_VERIFY;
	}
}
