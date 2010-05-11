/*
 *  GlIbo.h
 *  Crag
 *
 *  Created by John on 12/11/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "BufferObject.h"


namespace gl
{
	class Ibo : public BufferObject<GLuint, GL_ELEMENT_ARRAY_BUFFER_ARB>
	{
		typedef BufferObject<GLuint, GL_ELEMENT_ARRAY_BUFFER_ARB> BaseClass;
	public:
	};


	template<> inline GLuint Init<GL_ELEMENT_ARRAY_BUFFER_ARB>()
	{
		GLuint id;
		GLPP_CALL(glGenBuffers(1, &id));
		assert(id != 0);
		return id;
	}

	template<> inline void Deinit<GL_ELEMENT_ARRAY_BUFFER_ARB>(GLuint id)
	{
		assert (id != 0);
		GLPP_CALL(glDeleteBuffers(1, &id));
	}

	template<> inline void Bind<GL_ELEMENT_ARRAY_BUFFER_ARB>(GLuint id)
	{
		GLPP_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, id));
	}

	template<> inline GLuint GetBindingEnum<GL_ELEMENT_ARRAY_BUFFER_ARB>()
	{
		return GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB;
	}
}
