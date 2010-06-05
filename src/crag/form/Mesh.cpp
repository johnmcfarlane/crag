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
#include "form/Point.h"

#include "core/VectorOps.h"


form::Mesh::Mesh(int max_num_verts, int max_num_tris)
: vertices(max_num_verts)
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

form::Vertex & form::Mesh::GetVertex(Point & point, Vector2f const & texture)
{
	if (point.vert == nullptr)
	{
		point.vert = & AddVertex(point, texture);
	}
	
	Assert(point == point.vert->pos);
	//Assert(texture == point.vert->texture);
	
	return * point.vert;
}

form::Vertex & form::Mesh::AddVertex(form::Point const & p, Vector2f const & texture)
{
	Vertex & addition = vertices.PushBack();
	
	addition.pos = p;
	addition.norm = addition.norm.Zero();
#if defined(FORM_VERTEX_TEXTURE)
	addition.texture = texture;
#endif

	return addition;
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
	Assert(NearEqual(LengthSq(normal), 1.f, 0.01f));
	
	indices.PushBack(vertices.GetIndex(a));
	indices.PushBack(vertices.GetIndex(b));
	indices.PushBack(vertices.GetIndex(c));
	
	a.norm += normal;
	b.norm += normal;
	c.norm += normal;
}

/*void form::Mesh::AddFace(Vertex & a, Vertex & b, Vertex & c)
{
	Vector3f normal = TriangleNormal(a.pos, b.pos, c.pos);
	AddFace(a, b, c, FastNormalize(normal));
}*/

void form::Mesh::AddFace(Point & a, Point & b, Point & c, int ia, Vector3f const & normal)
{
	static const float low = 0.01f, high = .99f;
	static const float uv[3][2] = 
	{
		{ low, low },
		{ high, low },
		{ low, high }
	};
	
	Vertex & vert_a = GetVertex(a, Vector2f(uv[       ia   ][0],uv[       ia   ][1]));
	Vertex & vert_b = GetVertex(b, Vector2f(uv[TriMod(ia+1)][0],uv[TriMod(ia+1)][1]));
	Vertex & vert_c = GetVertex(c, Vector2f(uv[TriMod(ia+2)][0],uv[TriMod(ia+2)][1]));
	
	AddFace(vert_a, vert_b, vert_c, normal);
}

void form::Mesh::AddFace(Point & a, Point & b, Point & c, int ia)
{
	Vector3f normal = TriangleNormal(static_cast<Vector3f const &>(a), 
									 static_cast<Vector3f const &>(b), 
									 static_cast<Vector3f const &>(c));
	AddFace(a, b, c, ia, FastNormalize(normal));
}

form::VertexBuffer & form::Mesh::GetVertices() 
{
	return vertices;
}

form::VertexBuffer const & form::Mesh::GetVertices() const
{
	return vertices;
}

gfx::IndexBuffer & form::Mesh::GetIndices()
{
	return indices;
}

gfx::IndexBuffer const & form::Mesh::GetIndices() const
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
