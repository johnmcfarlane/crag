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

#include "core/memory.h"
#include "core/VectorOps.h"


// TODO: Add dirty normal flag?
void form::VertexBuffer::ClearNormals()
{
	// Clear the normals.
	form::Vertex * verts = GetArray();
	int num = GetMaxUsed();
	form::Vertex const * end = verts + num;
	
	// prefetch the norm
	for (form::Vertex * iterator = verts; iterator < end; ++ iterator)
	{
		iterator->norm = iterator->norm.Zero();
	}
}

void form::VertexBuffer::NormalizeNormals()
{
	// Clear the normals.
	form::Vertex * verts = GetArray();
	int num = GetMaxUsed();
	form::Vertex const * end = verts + num;
	
	// prefetch the norm
	for (form::Vertex * iterator = verts; iterator < end; ++ iterator)
	{
		FastNormalize(iterator->norm);
	}
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
