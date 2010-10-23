/*
 *  VertexBuffer.cpp
 *  Crag
 *
 *  Created by John on 3/3/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "VertexBuffer.h"

#include "sys/memory.h"
#include "geom/VectorOps.h"


form::VertexBuffer::VertexBuffer(int max_num_verts) 
: Super(max_num_verts)
{
}

form::Vertex & form::VertexBuffer::PushBack()
{
	return grow(1);
}

void form::VertexBuffer::Clear()
{
	clear();
}

void form::VertexBuffer::NormalizeNormals()
{
	const_iterator end_iterator = Super::end();
	for (iterator i = begin(); i != end_iterator; ++ i)
	{
		Vector3f & normal = i->norm;
		FastNormalize(normal);
	}
}

int form::VertexBuffer::GetIndex(Vertex const & v) const
{
	Vertex const * array = & front();

	int index = & v - array;
	
	return index;
}

/*#if DUMP 
DUMP_OPERATOR_DEFINITION(form, VertexBuffer)
{
	for (int i = 0; i < rhs.GetMaxUsed(); ++ i) {
		form::Vertex const & vertex = rhs[i];
		
		if (rhs.IsFree(vertex)) {
			continue;
		}
		
		lhs << i;
		lhs << " : " << vertex.pos.x << ',' << vertex.pos.y << ',' << vertex.pos.z;
		lhs << " : " << vertex.norm.x << ',' << vertex.norm.y << ',' << vertex.norm.z;
		lhs << '\n';
	}
	
	return lhs;
}
#endif*/
