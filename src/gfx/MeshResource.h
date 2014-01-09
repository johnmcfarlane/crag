//
//  MeshResource.h
//  crag
//
//  Created by John McFarlane on 2014-01-07.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Mesh.h"
#include "Model.h"

#include "geom/Intersection.h"

namespace gfx
{
	// a mesh stored in ResourceManager
	template <typename Vertex>
	class MeshResource final : public Model
	{
	public:
		// types
		typedef gfx::Mesh<Vertex> Mesh;
		
		// functions
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(MeshResource, self)
			CRAG_VERIFY(self._mesh);
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END

		MeshResource() = default;

		MeshResource(Mesh const && rhs)
		: _mesh(std::move(rhs))
		{
			CRAG_VERIFY(* this);
		}

		MeshResource(int num_vertices, int num_indices)
		: _mesh(num_vertices, num_indices)
		{
			CRAG_VERIFY(* this);
		}

		MeshResource(MeshResource const &) = delete;
		MeshResource & operator=(MeshResource const &) = delete;
		
		Mesh const & GetMesh() const
		{
			return _mesh;
		}
		
		Mesh & GetMesh()
		{
			return _mesh;
		}
		
	private:
		// variables
		Mesh _mesh;
	};
}
