/*
 *  BufferObject.h
 *  Crag
 *
 *  Created by John on 11/27/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Name.h"


namespace gl
{
	
	
#if defined(WIN32) 	
	// ARB works best on WIN32  but non-ARB version works with later hw
	inline void BindBuffer(GLenum target, GLuint buffer) 
	{ 
		glBindBufferARB(target, buffer); 
	}
	
	inline void BufferData(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage) 
	{ 
		glBufferDataARB(target, size, data, usage); 
	}
	
	inline void BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data)
	{
		glBufferSubDataARB(target, offset, size, data);
	}
	
	inline void DeleteBuffers(GLsizei n, const GLuint * buffers)
	{
		glDeleteBuffersARB(n, buffers);
	}
	
	inline void GenBuffers(GLsizei n, const GLuint * buffers)
	{
		glGenBuffersARB(n, buffers);
	}
	
	enum
	{
		ARRAY_BUFFER = GL_ARRAY_BUFFER_ARB, 
		ARRAY_BUFFER_BINDING = GL_ARRAY_BUFFER_BINDING_ARB, 
		ELEMENT_ARRAY_BUFFER = GL_ELEMENT_ARRAY_BUFFER_ARB, 
		ELEMENT_ARRAY_BUFFER_BINDING = GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB, 
		STATIC_DRAW = GL_STATIC_DRAW_ARB, 
		DYNAMIC_DRAW = GL_DYNAMIC_DRAW_ARB, 
		STREAM_DRAW = GL_STREAM_DRAW_ARB
	};
#else
	// ARB works best on WIN32  but non-ARB version works with later hw
	inline void BindBuffer(GLenum target, GLuint buffer) 
	{ 
		glBindBuffer(target, buffer); 
	}
	
	inline void BufferData(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage) 
	{ 
		glBufferData(target, size, data, usage); 
	}
	
	inline void BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data)
	{
		glBufferSubData(target, offset, size, data);
	}
	
	inline void DeleteBuffers(GLsizei n, const GLuint * buffers)
	{
		glDeleteBuffers(n, buffers);
	}
	
	inline void GenBuffers(GLsizei n, GLuint * buffers)
	{
		glGenBuffers(n, buffers);
	}
	
	enum
	{
		ARRAY_BUFFER = GL_ARRAY_BUFFER, 
		ARRAY_BUFFER_BINDING = GL_ARRAY_BUFFER_BINDING, 
		ELEMENT_ARRAY_BUFFER = GL_ELEMENT_ARRAY_BUFFER, 
		ELEMENT_ARRAY_BUFFER_BINDING = GL_ELEMENT_ARRAY_BUFFER_BINDING, 
		STATIC_DRAW = GL_STATIC_DRAW, 
		DYNAMIC_DRAW = GL_DYNAMIC_DRAW, 
		STREAM_DRAW = GL_STREAM_DRAW
	};
#endif
	
	// Base class for IBO and VBO.
	template <typename ELEMENT, GLenum TARGET> class BufferObject : public Name<TARGET>
	{
		typedef Name<TARGET> BaseClass;
	public:
		void Resize(size_t _num)
		{
			assert(BaseClass::IsBound());
			GLsizeiptr size = sizeof(ELEMENT) * _num;
			GLPP_CALL(gl::BufferData(TARGET, size, nullptr, gl::STREAM_DRAW));
		}
		
		void Set(size_t _num, ELEMENT const * array)
		{
			assert(BaseClass::IsBound());
			GLsizeiptr size = sizeof(ELEMENT) * _num;
			GLPP_CALL(gl::BufferSubData(TARGET, 0, size, array));
		}
		
		void Clear()
		{
			assert(BaseClass::IsBound());
			GLPP_CALL(gl::BufferData(TARGET, 0, nullptr, gl::STREAM_DRAW));
		}
	};

}
