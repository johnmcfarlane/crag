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


namespace gl
{
	class Ibo : public BufferObject<GLuint, gl::ELEMENT_ARRAY_BUFFER>
	{
		typedef BufferObject<GLuint, gl::ELEMENT_ARRAY_BUFFER> BaseClass;
	public:
	};


	template<> inline GLuint Init<gl::ELEMENT_ARRAY_BUFFER>()
	{
		GLuint id;
		GLPP_CALL(gl::GenBuffers(1, &id));
		assert(id != 0);
		return id;
	}

	template<> inline void Deinit<gl::ELEMENT_ARRAY_BUFFER>(GLuint id)
	{
		assert (id != 0);
		GLPP_CALL(gl::DeleteBuffers(1, &id));
	}

	template<> inline void Bind<gl::ELEMENT_ARRAY_BUFFER>(GLuint id)
	{
		GLPP_CALL(gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, id));
	}

	template<> inline GLuint GetBindingEnum<gl::ELEMENT_ARRAY_BUFFER>()
	{
		return gl::ELEMENT_ARRAY_BUFFER_BINDING;
	}
}
