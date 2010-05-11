/*
 *  Mesh.h
 *  Crag
 *
 *  Created by John on 10/24/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "form/VertexBuffer.h"
#include "gfx/IndexBuffer.h"
#include "core/debug.h"


namespace form
{
	DUMP_OPERATOR_DECLARATION(Mesh);
	
	class Mesh
	{
	public:
		Mesh(int max_num_tris, VertexBuffer const * init_vertices = nullptr);

		int GetIndexCount() const;
		int GetNumPolys() const;
		
		void SetVertices(VertexBuffer const * v);
		void ClearPolys();
		void AddFace(Vertex & a, Vertex & b, Vertex & c, Vector3f const & normal);
		void AddFace(Vertex & a, Vertex & b, Vertex & c);

		VertexBuffer const * GetVertices() const;
		IndexBuffer const & GetIndices() const;
		
#if VERIFY
		void Verify() const;
#endif
		
		DUMP_OPERATOR_FRIEND_DECLARATION(Mesh);
		
	private:
		VertexBuffer const * vertices;
		IndexBuffer indices;
	};

}


#if VERIFY
void Verify(form::Mesh const &);
#endif
