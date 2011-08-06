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

#include "form/defs.h"

#include "MeshProperties.h"

#include "glpp/Mesh.h"
#include "glpp/Vertex.h"


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
		typedef gl::Mesh<Vertex> super;
	public:
		MeshBufferObject();
		
		void Set(form::Mesh const & mesh);
		int GetNumPolys() const;
		
		void Activate(gfx::Pov pov) const;
		void Deactivate() const;
		
		void Draw() const;
		
	private:
		void SetVbo(VertexBuffer const & vertices);	
		void SetIbo(gfx::IndexBuffer const & indices);
		
		int max_index;
		MeshProperties properties;
	};
}
