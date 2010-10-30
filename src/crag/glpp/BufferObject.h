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


#if defined(WIN32) 
// works best on WIN32  but non-ARB version works with later hw
#define glppBindBuffer						glBindBufferARB
#define glppBufferData						glBufferDataARB
#define glppBufferSubData					glBufferSubDataARB
#define glppDeleteBuffers					glDeleteBuffersARB
#define glppGenBuffers						glGenBuffersARB
#define GLPP_ARRAY_BUFFER					GL_ARRAY_BUFFER_ARB
#define GLPP_ARRAY_BUFFER_BINDING			GL_ARRAY_BUFFER_BINDING_ARB 
#define GLPP_ELEMENT_ARRAY_BUFFER			GL_ELEMENT_ARRAY_BUFFER_ARB
#define GLPP_ELEMENT_ARRAY_BUFFER_BINDING	GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB
#define GLPP_STATIC_DRAW					GL_STATIC_DRAW_ARB
#define GLPP_DYNAMIC_DRAW					GL_DYNAMIC_DRAW_ARB 
#define GLPP_STREAM_DRAW					GL_STREAM_DRAW_ARB 
#else
#define glppBindBuffer						glBindBuffer
#define glppBufferData						glBufferData
#define glppBufferSubData					glBufferSubData
#define glppDeleteBuffers					glDeleteBuffers
#define glppGenBuffers						glGenBuffers
#define GLPP_ARRAY_BUFFER					GL_ARRAY_BUFFER
#define GLPP_ARRAY_BUFFER_BINDING			GL_ARRAY_BUFFER_BINDING
#define GLPP_ELEMENT_ARRAY_BUFFER			GL_ELEMENT_ARRAY_BUFFER
#define GLPP_ELEMENT_ARRAY_BUFFER_BINDING	GL_ELEMENT_ARRAY_BUFFER_BINDING
#define GLPP_STATIC_DRAW					GL_STATIC_DRAW
#define GLPP_DYNAMIC_DRAW					GL_DYNAMIC_DRAW
#define GLPP_STREAM_DRAW					GL_STREAM_DRAW
#endif


namespace gl
{
	
	// Base class for IBO and VBO.
	template <typename ELEMENT, GLenum TARGET> class BufferObject : public Name<TARGET>
	{
		typedef Name<TARGET> BaseClass;
	public:
		void Resize(int _num)
		{
			assert(BaseClass::IsBound());
			GLsizeiptr size = sizeof(ELEMENT) * _num;
			GLPP_CALL(glppBufferData(TARGET, size, nullptr, GLPP_STREAM_DRAW));
		}
		
		void Set(int _num, ELEMENT const * array)
		{
			assert(BaseClass::IsBound());
			GLsizeiptr size = sizeof(ELEMENT) * _num;
			GLPP_CALL(glppBufferSubData(TARGET, 0, size, array));
		}
		
		void Clear()
		{
			assert(BaseClass::IsBound());
			GLPP_CALL(glppBufferData(TARGET, 0, nullptr, GLPP_STREAM_DRAW));
		}
	};

}
