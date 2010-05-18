/*
 *  GlIbo.h
 *  Crag
 *
 *  Created by John on 12/11/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "BufferObject.h"


#if ! defined(WIN32)
#define GLPP_ELEMENT_ARRAY_BUFFER			GL_ELEMENT_ARRAY_BUFFER
#define GLPP_ELEMENT_ARRAY_BUFFER_BINDING	GL_ELEMENT_ARRAY_BUFFER_BINDING
#else
// works well on WIN32
#define GLPP_ELEMENT_ARRAY_BUFFER			GL_ELEMENT_ARRAY_BUFFER_ARB
#define GLPP_ELEMENT_ARRAY_BUFFER_BINDING	GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB
#endif

namespace gl
{
	class Ibo : public BufferObject<GLuint, GLPP_ELEMENT_ARRAY_BUFFER>
	{
		typedef BufferObject<GLuint, GLPP_ELEMENT_ARRAY_BUFFER> BaseClass;
	public:
	};


	template<> inline GLuint Init<GLPP_ELEMENT_ARRAY_BUFFER>()
	{
		GLuint id;
		GLPP_CALL(glppGenBuffers(1, &id));
		assert(id != 0);
		return id;
	}

	template<> inline void Deinit<GLPP_ELEMENT_ARRAY_BUFFER>(GLuint id)
	{
		assert (id != 0);
		GLPP_CALL(glppDeleteBuffers(1, &id));
	}

	template<> inline void Bind<GLPP_ELEMENT_ARRAY_BUFFER>(GLuint id)
	{
		GLPP_CALL(glppBindBuffer(GLPP_ELEMENT_ARRAY_BUFFER, id));
	}

	template<> inline GLuint GetBindingEnum<GLPP_ELEMENT_ARRAY_BUFFER>()
	{
		return GLPP_ELEMENT_ARRAY_BUFFER_BINDING;
	}
}
