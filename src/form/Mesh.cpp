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

void Mesh::Reserve(int max_num_verts, int max_num_tris)
{
	_lit_mesh.GetVertices().reserve(max_num_verts);
	_lit_mesh.GetIndices().reserve(max_num_tris);
}

void Mesh::Clear()
{
	_lit_mesh.Clear();
}

MeshProperties & Mesh::GetProperties()
{
	return properties;
}

MeshProperties const & Mesh::GetProperties() const
{
	return properties;
}

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
	// TODO: if contour lines stay, move height into Point
	auto calculate_height = [] (geom::rel::Vector3 const & pos, geom::rel::Vector3 const & origin) -> float
	{
		using namespace geom;
		auto origin_height = Length(origin);
		auto relative_center = pos + origin;
		auto height = Length(relative_center);
		auto relative_height = height - origin_height;
		return relative_height;
	};
	
	Vertex v = 
	{ 
		p.pos, 
		Vertex::Vector3::Zero(),
		gfx::Color4b(color.r,
					 color.g,
					 color.b,
					 color.a),
		calculate_height(p.pos, geom::Cast<float>(properties._origin))
	};

	auto & vertices = _lit_mesh.GetVertices();
	vertices.push_back(v);
	
	return vertices.back();
}

void Mesh::AddFace(Vertex & a, Vertex & b, Vertex & c, Vertex::Vector3 const & normal)
{
	ASSERT(NearEqual(LengthSq(normal), 1.f, 0.01f));
	
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

Mesh::LitMesh const & Mesh::GetLitMesh() const
{
	return _lit_mesh;
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Mesh, self)
	// slow
	//CRAG_VERIFY(self._lit_mesh);
CRAG_VERIFY_INVARIANTS_DEFINE_END
