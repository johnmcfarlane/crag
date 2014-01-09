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
#include "gfx/IndexedVboResource.h"
#include "gfx/LitVertex.h"

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
	template<typename VERTEX, GLenum USAGE> class IndexedVboResource;

	typedef gfx::IndexedVboResource<Vector3, GL_DYNAMIC_DRAW> ShadowVolumeResource;

	// An OpenGL vertex/index buffer pair specialized for storing the results of the formation system for rendering.
	class FormationGeneration
	{
	public:
		// types
		typedef gfx::IndexedVboResource<LitVertex, GL_DYNAMIC_DRAW> IndexedVboResource;

		// functions
		CRAG_VERIFY_INVARIANTS_DECLARE(FormationGeneration);

		FormationGeneration(int max_num_quaterne);
		
		void SetMesh(form::Mesh const & mesh);

		IndexedVboResource const & GetVboResource() const;
		
		geom::abs::Vector3 const & GetOrigin() const;
		
		void Activate() const;
		void Deactivate() const;
		
		void Draw() const;
		
	private:
		// variables
		IndexedVboResource _indexed_vbo_resource;
		form::MeshProperties _properties;
	};
}
