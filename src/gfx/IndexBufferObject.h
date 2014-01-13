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
		IndexBufferObject() = default;

		IndexBufferObject(IndexBufferObject && rhs)
			: super(std::move(rhs))
		{
		}

		IndexBufferObject & operator=(IndexBufferObject && rhs)
		{
			super::operator=(std::move(rhs));
			return * this;
		}

		void DrawElements(GLenum mode, GLsizei count, GLuint first = 0) const
		{
			ASSERT(IsBound());
			CRAG_VERIFY_OP(static_cast<uintmax_t>(count), <=, static_cast<uintmax_t>(std::numeric_limits<ElementIndex>::max()));
			GLvoid const * ptr = reinterpret_cast<const GLvoid *>(sizeof(ElementIndex) * first);
			GL_CALL(glDrawElements(mode, count, CRAG_GFX_ELEMENT_INDEX_TYPE, ptr));
		}
	};
}
