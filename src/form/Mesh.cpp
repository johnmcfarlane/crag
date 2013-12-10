//
//  Mesh.cpp
//  crag
//
//  Created by John on 10/24/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Mesh.h"

#include "form/Point.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"

using namespace form;

////////////////////////////////////////////////////////////////////////////////
// form::Mesh

Mesh::Mesh(int max_num_verts, int max_num_tris)
: vertices(max_num_verts)
, indices(max_num_tris * 3)
{
}

void Mesh::Clear()
{
	indices.Clear();
	vertices.Clear();
}

MeshProperties & Mesh::GetProperties()
{
	return properties;
}

MeshProperties const & Mesh::GetProperties() const
{
	return properties;
}

size_t Mesh::GetIndexCount() const
{
	return indices.GetSize();
}

size_t Mesh::GetNumPolys() const
{
	return indices.GetSize() / 3;
}

Vertex & Mesh::GetVertex(Point & point, Color color)
{
	if (point.vert == nullptr)
	{
		point.vert = & AddVertex(point, color);
	}
	
	ASSERT(point.pos == point.vert->pos);
	//ASSERT(texture == point.vert->texture);
	
	return * point.vert;
}

Vertex & Mesh::AddVertex(Point const & p, Color color)
{
	Vertex v = 
	{ 
		p.pos, 
		Vector3::Zero(),
		gfx::Color4b(color.r,
					 color.g,
					 color.b,
					 color.a)
	};

	Vertex & addition = vertices.PushBack(v);
	
	return addition;
}

void Mesh::AddFace(Vertex & a, Vertex & b, Vertex & c, geom::Vector3f const & normal)
{
	ASSERT(NearEqual(LengthSq(normal), 1.f, 0.01f));
	
	gfx::IndexBuffer::value_type * corner_indices = & indices.PushBack();
	corner_indices [0] = vertices.GetIndex(a);
	corner_indices [1] = vertices.GetIndex(b);
	corner_indices [2] = vertices.GetIndex(c);

	// These additions are not thread-safe but I can't imagine it causing more
	// than a minor inaccuracy in the normal calculation. 
	a.norm += normal;
	b.norm += normal;
	c.norm += normal;
}

void Mesh::AddFace(Point & a, Point & b, Point & c, geom::Vector3f const & normal, gfx::Color4b color)
{
	Vertex & vert_a = GetVertex(a, color);
	Vertex & vert_b = GetVertex(b, color);
	Vertex & vert_c = GetVertex(c, color);
	
	AddFace(vert_a, vert_b, vert_c, normal);
}

VertexBuffer & Mesh::GetVertices() 
{
	return vertices;
}

VertexBuffer const & Mesh::GetVertices() const
{
	return vertices;
}

gfx::IndexBuffer & Mesh::GetIndices()
{
	return indices;
}

gfx::IndexBuffer const & Mesh::GetIndices() const
{
	return indices;
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Mesh, self)
	CRAG_VERIFY_EQUAL(self.indices.GetSize() % 3, 0);
	CRAG_VERIFY_EQUAL(self.indices.GetSlack() % 3, 0);

	for (auto index : self.indices)
	{
		Vertex const & v = self.vertices[index];
		CRAG_VERIFY_ARRAY_ELEMENT(& v, std::begin(self.vertices), std::end(self.vertices));
		CRAG_VERIFY(v);
	}
CRAG_VERIFY_INVARIANTS_DEFINE_END
