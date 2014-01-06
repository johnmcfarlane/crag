//
//  FormationGeneration.h
//  crag
//
//  Created by John on 12/4/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "gfx/defs.h"
#include "gfx/MeshResource.h"
#include "gfx/Vertex.h"

#include "form/Mesh.h"
#include "form/MeshProperties.h"

#include "geom/origin.h"

namespace form
{
	// forward-declarations
	class IndexBuffer;
	class Mesh;
	class VertexBuffer;
}
	
namespace gfx 
{ 
	// forward-declarations
	template<typename VERTEX, GLenum USAGE> class MeshResource;

	typedef gfx::MeshResource<Vector3, GL_DYNAMIC_DRAW> ShadowVolumeResource;

	// An OpenGL vertex/index buffer pair specialized for storing the results of the formation system for rendering.
	class FormationGeneration
	{
	public:
		// types
		typedef gfx::MeshResource<Vertex, GL_DYNAMIC_DRAW> MeshResource;

		// functions
		CRAG_VERIFY_INVARIANTS_DECLARE(FormationGeneration);

		FormationGeneration(int max_num_quaterne);
		
		void SetMesh(form::Mesh const & mesh);

		MeshResource const & GetVboResource() const;
		
		geom::abs::Vector3 const & GetOrigin() const;
		
		void Activate() const;
		void Deactivate() const;
		
		void Draw() const;
		
	private:
		// variables
		MeshResource _mesh_resource;
		form::MeshProperties _properties;
	};
}
