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


#if defined(WIN32) && 0
#define glppBindBuffer		glBindBuffer
#define glppBufferData		glBufferData
#define glppBufferSubData	glBufferSubData
#define glppDeleteBuffers	glDeleteBuffers
#define glppGenBuffers		glGenBuffers
#else
#define glppBindBuffer		glBindBufferARB
#define glppBufferData		glBufferDataARB
#define glppBufferSubData	glBufferSubDataARB
#define glppDeleteBuffers	glDeleteBuffersARB
#define glppGenBuffers		glGenBuffersARB
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
			GLPP_CALL(glppBufferData(TARGET, size, nullptr, GL_STATIC_DRAW_ARB));
		}
		
		void Set(int _num, ELEMENT const * array)
		{
			assert(BaseClass::IsBound());
			GLsizeiptr size = sizeof(ELEMENT) * _num;
			GLPP_CALL(glppBufferSubData(TARGET, 0, size, array));
		}
	};

}
