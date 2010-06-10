/*
 *  Mesh.h
 *  Crag
 *
 *  Created by John on 10/24/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "form/VertexBuffer.h"
#include "gfx/IndexBuffer.h"
#include "core/debug.h"


namespace form
{
	class Point;
	
	DUMP_OPERATOR_DECLARATION(Mesh);
	
	class Mesh
	{
	public:
		Mesh(int max_num_verts, int max_num_tris);

		int GetIndexCount() const;
		int GetNumPolys() const;
		
		Vertex & GetVertex(Point & point);
	private:
		Vertex & AddVertex(Point const & p);
	public:
		
		void ClearPolys();
		void AddFace(Vertex & a, Vertex & b, Vertex & c, Vector3f const & normal);
		//void AddFace(Vertex & a, Vertex & b, Vertex & c);
		void AddFace(Point & a, Point & b, Point & c, Vector3f const & normal);
		void AddFace(Point & a, Point & b, Point & c);

		VertexBuffer & GetVertices();
		VertexBuffer const & GetVertices() const;
		
		gfx::IndexBuffer & GetIndices();
		gfx::IndexBuffer const & GetIndices() const;
		
#if VERIFY
		void Verify() const;
#endif
		
		DUMP_OPERATOR_FRIEND_DECLARATION(Mesh);
		
	private:
		VertexBuffer vertices;
		gfx::IndexBuffer indices;
	};

}


#if VERIFY
void Verify(form::Mesh const &);
#endif
