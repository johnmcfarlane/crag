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

#include "form/scene/Mesh.h"
#include "form/node/Point.h"

#include "geom/VectorOps.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"


namespace  
{
	CONFIG_DEFINE (init_flat_shaded, bool, false);
}

form::Mesh::Mesh(int max_num_verts, int max_num_tris)
: vertices(max_num_verts)
, indices(max_num_tris * 3)
, flat_shaded(init_flat_shaded)
{
}

void form::Mesh::SetFlatShaded(bool fs)
{
	flat_shaded = fs;
}

bool form::Mesh::GetFlatShaded() const
{
	return flat_shaded;
}

int form::Mesh::GetIndexCount() const
{
	return indices.GetSize();
}

int form::Mesh::GetNumPolys() const
{
	return indices.GetSize() / 3;
}

form::Vertex & form::Mesh::GetVertex(Point & point)
{
	if (point.vert == nullptr)
	{
		point.vert = & AddVertex(point);
	}
	
	Assert(point.pos == point.vert->pos);
	//Assert(texture == point.vert->texture);
	
	return * point.vert;
}

form::Vertex & form::Mesh::AddVertex(form::Point const & p)
{
	Vertex & addition = vertices.PushBack();
	
	addition.pos = p.pos;
	addition.norm = addition.norm.Zero();

#if defined(FORM_VERTEX_TEXTURE)
	addition.texture = p.texture_uv;
#endif

#if defined(FORM_VERTEX_COLOR)
	//addition.color = gfx::Color4b(Random::sequence.GetInt(256), Random::sequence.GetInt(256), Random::sequence.GetInt(256), Random::sequence.GetInt(256));
	addition.color = p.col;
#endif

	return addition;
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

void form::Mesh::AddFace(Point & a, Point & b, Point & c, Vector3f const & normal)
{
	Vertex * vert_a, * vert_b, * vert_c;
	
	vert_a = & GetVertex(a);
	vert_b = & GetVertex(b);
	if (! flat_shaded)
	{
		vert_c = & GetVertex(c);
	}
	else 
	{
		vert_c = & AddVertex(c);
	}
	
	AddFace(* vert_a, * vert_b, * vert_c, normal);
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
