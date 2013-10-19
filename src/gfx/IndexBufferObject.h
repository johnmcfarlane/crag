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
	class IndexBufferObject : public BufferObject<ElementIndex, GL_ELEMENT_ARRAY_BUFFER>
	{
		typedef BufferObject<ElementIndex, GL_ELEMENT_ARRAY_BUFFER> super;
	public:
		void DrawElements(GLenum mode, GLsizei count, GLuint first = 0) const
		{
			ASSERT(IsBound());
			ASSERT(count <= std::numeric_limits<GLsizei>::max());
			GLvoid const * ptr = reinterpret_cast<const GLvoid *>(sizeof(ElementIndex) * first);
			GL_CALL(glDrawElements(mode, count, CRAG_GFX_ELEMENT_INDEX_TYPE, ptr));
		}
	};
}
