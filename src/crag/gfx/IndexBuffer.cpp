/*
 *  IndexBuffer.cpp
 *  Crag
 *
 *  Created by john on 5/3/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "gfx/IndexBuffer.h"

//#include "form/VertexBuffer.h"

#include "core/VectorOps.h"
#include "core/memory.h"


//////////////////////////////////////////////////////////////////////
// IndexBuffer member definitions

IndexBuffer::IndexBuffer(int buffer_capacity)
{
	reserve(buffer_capacity);
}

void IndexBuffer::Clear()
{
	size_t s = size();
	if (s > 0) {
		GLuint & beginning = * begin();
		ZeroArray(& beginning, s);
		
		clear();
	}
}

IndexBuffer::value_type const * IndexBuffer::GetArray() const
{
	return & front();
}
