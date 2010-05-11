/*
 *  form/MeshBufferObject.h
 *  Crag
 *
 *  Created by John on 12/4/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "glpp/Mesh.h"
#include "form/Vertex.h"


class IndexBuffer;


namespace form
{
	class Mesh;
	class VertexBuffer;

	class MeshBufferObject : public gl::Mesh<Vertex>
	{
	public:
		MeshBufferObject();
		
		void Set(form::Mesh const & mesh);
		
		void BeginDraw(bool color);
		void EndDraw();
		
		void Draw();
		
	private:
		void SetVbo(form::VertexBuffer const & vertices);	
		void SetIbo(IndexBuffer const & indices);
		
		int max_index;
	};
}
