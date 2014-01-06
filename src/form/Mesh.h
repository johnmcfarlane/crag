//
//  Mesh.h
//  crag
//
//  Created by John on 10/24/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "IndexBuffer.h"
#include "MeshProperties.h"
#include "VertexBuffer.h"

namespace form
{
	// forward-declaration
	class Point;
	
	// Stores a complete mesh of the formations.
	// Is generated by the NodeBuffer and then passed to a VertexBufferObject for rendering.
	class Mesh
	{
	public:
		// types
		typedef gfx::Vertex Vertex;
		typedef Vertex::Color Color;
		
		// functions
		Mesh(int max_num_verts, int max_num_tris);
		
		void Clear();
		
		MeshProperties & GetProperties();
		MeshProperties const & GetProperties() const;

		Vertex & GetVertex(Point & point, Color color);
	private:
		Vertex & AddVertex(Point const & p, Color color);
	public:
		
		void AddFace(Vertex & a, Vertex & b, Vertex & c, Vertex::Vector3 const & normal);
		void AddFace(Point & a, Point & b, Point & c, Vertex::Vector3 const & normal, gfx::Color4b color);

		VertexBuffer & GetVertices();
		VertexBuffer const & GetVertices() const;
		
		IndexBuffer & GetIndices();
		IndexBuffer const & GetIndices() const;
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Mesh);
		
		// variables
	private:
		VertexBuffer vertices;
		IndexBuffer indices;
		MeshProperties properties;
	};
	
}
