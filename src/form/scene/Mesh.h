//
//  Mesh.h
//  crag
//
//  Created by John on 10/24/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "form/defs.h"

#include "MeshProperties.h"
#include "VertexBuffer.h"

#include "gfx/IndexBuffer.h"


namespace form
{
	// forward-declaration
	class Point;
	
	// Stores a complete mesh of the formations.
	// Is generated by the NodeBuffer and then passed to a VertexBufferObject for rendering.
	class Mesh
	{
	public:
		// functions
		Mesh(int max_num_verts, int max_num_tris);
		
		void Clear();
		
		MeshProperties & GetProperties();
		MeshProperties const & GetProperties() const;

		size_t GetIndexCount() const;
		size_t GetNumPolys() const;
		
		Vertex & GetVertex(Point & point, Color color);
	private:
		Vertex & AddVertex(Point const & p, Color color);
	public:
		
		void AddFace(Vertex & a, Vertex & b, Vertex & c, Vector3f const & normal);
		void AddFace(Point & a, Point & b, Point & c, Vector3f const & normal, gfx::Color4b color);

		VertexBuffer & GetVertices();
		VertexBuffer const & GetVertices() const;
		
		gfx::IndexBuffer & GetIndices();
		gfx::IndexBuffer const & GetIndices() const;
		
#if defined(VERIFY)
		void Verify() const;
#endif
		
		DUMP_OPERATOR_FRIEND_DECLARATION(Mesh);
		
		// variables
	private:
		VertexBuffer vertices;
		gfx::IndexBuffer indices;
		MeshProperties properties;
		
	public:
		DEFINE_INTRUSIVE_LIST(Mesh, list_type);
	};
	
}
