//
//  MeshBufferObject.h
//  crag
//
//  Created by John on 12/4/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "MeshProperties.h"
#include "Vertex.h"

#include "gfx/Mesh.h"
#include "gfx/MeshResource.h"


namespace gfx 
{ 
	class IndexBuffer;
}


namespace form
{
	// forward-declarations
	class Mesh;
	class VertexBuffer;
	
	
	// An OpenGL vertex/index buffer pair specialized for storing the results of the formation system for rendering.
	// TODO: Does this class belong in gfx?
	// TODO: Clean up inheritance. Also maybe rename this class.
	class MeshBufferObject : public gfx::Mesh<Vertex, GL_DYNAMIC_DRAW>, public gfx::MeshResource
	{
		typedef gfx::Mesh<Vertex, GL_DYNAMIC_DRAW> super;
	public:
		MeshBufferObject();
		virtual ~MeshBufferObject();
		
		void Set(form::Mesh const & mesh);
		int GetNumPolys() const;
		MeshProperties::Vector const & GetOrigin() const;
		
		void Activate() const;
		void Deactivate() const;
		
		void Draw() const;
		
	private:
		void SetVbo(VertexBuffer const & vertices);	
		void SetIbo(gfx::IndexBuffer const & indices);
		
		int max_index;
		MeshProperties properties;
	};
}
