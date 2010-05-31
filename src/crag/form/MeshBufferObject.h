/*
 *  form/MeshBufferObject.h
 *  Crag
 *
 *  Created by John on 12/4/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "glpp/Mesh.h"
#include "form/Vertex.h"


namespace gfx { class IndexBuffer; }


namespace form
{
	class Mesh;
	class VertexBuffer;

	class MeshBufferObject : public gl::Mesh<Vertex>
	{
	public:
		MeshBufferObject();
		
		void Set(Mesh const & mesh);
		
		void BeginDraw(bool color);
		void EndDraw();
		
		void Draw();
		
	private:
		void SetVbo(VertexBuffer const & vertices);	
		void SetIbo(gfx::IndexBuffer const & indices);
		
		int max_index;
	};
}
