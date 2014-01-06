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

using namespace form;

////////////////////////////////////////////////////////////////////////////////
// form::VertexBuffer member definitions

VertexBuffer::VertexBuffer(int max_num_verts) 
: _verts(max_num_verts)
{
}

gfx::Vertex & VertexBuffer::PushBack(Vertex const & element)
{
	return _verts.push_back(element);
}

void VertexBuffer::Clear()
{
	_verts.clear();
}

int VertexBuffer::GetIndex(Vertex const & v) const
{
	Vertex const * array = & _verts.front();

	int index = int(& v - array);
	
	return index;
}

gfx::Vertex const & VertexBuffer::operator[] (int index) const
{
	return _verts[index];
}

gfx::Vertex & VertexBuffer::operator[] (int index)
{
	return _verts[index];
}
