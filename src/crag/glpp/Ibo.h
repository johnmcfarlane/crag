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
		typedef BufferObject<GLuint, gl::ELEMENT_ARRAY_BUFFER> super;
	public:
		bool IsBound() const
		{
			return super::IsBound();	// slightly silly
		}		
	};

	inline void DrawElements(Ibo const & ibo, GLenum mode, GLuint first, GLsizei count)
	{
		assert(ibo.IsBound());
		GLPP_CALL(glDrawElements (mode, count, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid *>(sizeof(GLuint) * first)));
	}
}
