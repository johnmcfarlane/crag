//
//  IndexBufferObject.h
//  crag
//
//  Created by John on 12/11/09.
//  Copyright 2009 - 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "BufferObject.h"


namespace gfx
{
	// Wrapper for OpenGL IBO using unsigned integer indices.
	class IndexBufferObject : public BufferObject<GLuint, GL_ELEMENT_ARRAY_BUFFER>
	{
		typedef BufferObject<GLuint, GL_ELEMENT_ARRAY_BUFFER> super;
	public:
		void DrawElements(GLenum mode, GLsizei count, GLuint first = 0) const
		{
			assert(IsBound());
			GLvoid const * ptr = reinterpret_cast<const GLvoid *>(sizeof(GLuint) * first);
			GL_CALL(glDrawElements(mode, count, GL_UNSIGNED_INT, ptr));
		}
	};
}
