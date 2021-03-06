//
//  form/Mesh.cpp
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

#if defined(CRAG_FORM_FLAT_SHADE)
void Mesh::Reserve(int, int max_num_tris)
{
	_lit_mesh.reserve(max_num_tris * 3);
}
#else
void Mesh::Reserve(int max_num_verts, int max_num_tris)
{
	_lit_mesh.GetVertices().reserve(max_num_verts);
	_lit_mesh.GetIndices().reserve(max_num_tris);
}
#endif

void Mesh::Clear()
{
	_lit_mesh.clear();
}

void Mesh::NormalizeNormals()
{
	for (auto & vertex : _lit_mesh)
	{
		Vector3 & norm = vertex.norm;
		geom::Normalize(norm);
	}
}

MeshProperties & Mesh::GetProperties()
{
	return properties;
}

MeshProperties const & Mesh::GetProperties() const
{
	return properties;
}

#if ! defined(CRAG_FORM_FLAT_SHADE)
Mesh::Vertex & Mesh::GetVertex(Point & point, Color color)
{
	if (point.vert == nullptr)
	{
		point.vert = & AddVertex(point, color);
	}
	
	ASSERT(point.pos == point.vert->pos);
	
	return * point.vert;
}

Mesh::Vertex & Mesh::AddVertex(Point const & p, Color color)
{
	Vertex v = 
	{ 
		p.pos, 
		Vertex::Vector3::Zero(),
		gfx::Color4b(color.r,
					 color.g,
					 color.b,
					 color.a)
	};

	auto & vertices = _lit_mesh.GetVertices();
	vertices.push_back(v);
	
	return vertices.back();
}

void Mesh::AddFace(Vertex & a, Vertex & b, Vertex & c, Vertex::Vector3 const & normal)
{
	ASSERT(NearEqual(MagnitudeSq(normal), 1.f, 0.01f));
	
	auto & vertices = _lit_mesh.GetVertices();
	auto & indices = _lit_mesh.GetIndices();

	auto add_corner = [&] (Vertex & vertex)
	{
		auto vertex_index = std::distance(& * vertices.begin(), & vertex);
		indices.push_back(vertex_index);
	};

	add_corner(a);
	add_corner(b);
	add_corner(c);

	// These additions are not thread-safe but I can't imagine it causing more
	// than a minor inaccuracy in the normal calculation. 
	a.norm += normal;
	b.norm += normal;
	c.norm += normal;
}

void Mesh::AddFace(Point & a, Point & b, Point & c, Vertex::Vector3 const & normal, gfx::Color4b color)
{
	Vertex & vert_a = GetVertex(a, color);
	Vertex & vert_b = GetVertex(b, color);
	Vertex & vert_c = GetVertex(c, color);
	
	AddFace(vert_a, vert_b, vert_c, normal);
}

#else

void Mesh::AddFace(Point const & a, Point const & b, Point const & c, Vertex::Vector3 const & normal, gfx::Color4b color)
{
	auto add_face = [&] (Point const & p)
	{
		_lit_mesh.push_back({ 
			p.pos, 
			normal,
			gfx::Color4b(color.r,
						 color.g,
						 color.b,
						 color.a)
		});
	};
	
	add_face(a);
	add_face(b);
	add_face(c);
}

#endif

Mesh::LitMesh const & Mesh::GetLitMesh() const
{
	return _lit_mesh;
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Mesh, self)
#if ! defined(CRAG_FORM_FLAT_SHADE)
	CRAG_VERIFY(self._lit_mesh);
#endif

	for (auto & vertex : self._lit_mesh)
	{
		auto & norm = vertex.norm;
		CRAG_VERIFY_UNIT(norm, .0001f);
	}
CRAG_VERIFY_INVARIANTS_DEFINE_END
