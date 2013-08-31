//
//  VertexBuffer.cpp
//  crag
//
//  Created by John on 3/3/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "VertexBuffer.h"

#include "core/memory.h"


form::VertexBuffer::VertexBuffer(int max_num_verts) 
: _verts(max_num_verts)
{
}

form::Vertex & form::VertexBuffer::PushBack(Vertex const & element)
{
	return _verts.push_back(element);
}

void form::VertexBuffer::Clear()
{
	_verts.clear();
}

int form::VertexBuffer::GetIndex(Vertex const & v) const
{
	Vertex const * array = & _verts.front();

	int index = int(& v - array);
	
	return index;
}
