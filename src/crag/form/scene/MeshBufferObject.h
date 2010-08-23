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
#include "form/scene/Vertex.h"


namespace gfx 
{ 
	class IndexBuffer; 
	class Pov;
}


namespace form
{
	// forward-declarations
	class Mesh;
	class VertexBuffer;
	
	
	// An OpenGL vertex/index buffer pair specialized for storing the results of the formation system for rendering.
	class MeshBufferObject : public gl::Mesh<Vertex>
	{
	public:
		MeshBufferObject();
		
		void Set(Mesh const & mesh, Vector3d const & origin, bool flat_shaded);
		int GetNumPolys() const;
		
		void BeginDraw(gfx::Pov pov, bool color);
		void EndDraw();
		
		void Draw();
		
	private:
		void SetVbo(VertexBuffer const & vertices);	
		void SetIbo(gfx::IndexBuffer const & indices);
		
		Vector3d origin;
		int max_index;
		bool flat_shaded;		
	};
}
