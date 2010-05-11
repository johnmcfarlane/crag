/*
 *  Mesh.cpp
 *  Crag
 *
 *  Created by John on 10/24/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "form/Mesh.h"

#include "core/VectorOps.h"


form::Mesh::Mesh(int max_num_tris, VertexBuffer const * init_vertices)
: vertices(init_vertices)
, indices(max_num_tris * 3)
{
}

int form::Mesh::GetIndexCount() const
{
	return indices.GetSize();
}

int form::Mesh::GetNumPolys() const
{
	return indices.GetSize() / 3;
}

void form::Mesh::SetVertices(VertexBuffer const * v)
{
	vertices = v;
	ClearPolys();
}

void form::Mesh::ClearPolys()
{
#if ! defined(NDEBUG)
	int max_num_tris = indices.GetCapacity();
#endif
	
	indices.Clear();
	
	Assert(max_num_tris == indices.GetCapacity());
}

void form::Mesh::AddFace(Vertex & a, Vertex & b, Vertex & c, Vector3f const & normal)
{
	// TODO: Critical section
	Assert(vertices != nullptr);
	
	Assert(NearEqual(LengthSq(normal), 1.f, 0.01f));
	
	indices.PushBack(vertices->GetIndex(a));
	indices.PushBack(vertices->GetIndex(b));
	indices.PushBack(vertices->GetIndex(c));
	
	a.norm += normal;
	b.norm += normal;
	c.norm += normal;
}

void form::Mesh::AddFace(Vertex & a, Vertex & b, Vertex & c)
{
	Vector3f normal = TriangleNormal(a.pos, b.pos, c.pos);
	AddFace(a, b, c, FastNormalize(normal));
}

form::VertexBuffer const * form::Mesh::GetVertices() const
{
	return vertices;
}

IndexBuffer const & form::Mesh::GetIndices() const
{
	return indices;
}

#if VERIFY
void form::Mesh::Verify() const
{
	::VerifyRef(vertices);
	
	VerifyTrue ((indices.GetSize() % 3) == 0);
	VerifyTrue ((indices.GetSlack() % 3) == 0);
}
#endif

#if DUMP
DUMP_OPERATOR_DEFINITION(form, Mesh)
{
	lhs << "Mesh:" << '\n';
	
	lhs << "vertices: " << rhs.vertices;
	lhs << lhs.NewLine() << '\n';
	lhs << "indices" << rhs.indices;
	return lhs;
}
#endif
