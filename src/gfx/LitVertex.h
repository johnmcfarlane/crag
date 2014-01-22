//
//  gfx/LitVertex.h
//  crag
//
//  Created by John McFarlane on 2012-01-23.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Color.h"
#include "defs.h"

namespace gfx
{
	// Vertex type used by gfx/form systems to generate OpenGL mesh.
	struct LitVertex
	{
		typedef ::gfx::Vector3 Vector3;
		typedef Color4b Color;
		
		CRAG_VERIFY_INVARIANTS_DECLARE(LitVertex);

		Vector3 pos;
		Vector3 norm;
		Color4b color;
		float height;
	};

	using LitMesh = Mesh<LitVertex, ElementIndex>;
	using LitMeshHandle = crag::core::ResourceHandle<LitMesh>;
	using LitVboResource = IndexedVboResource<LitVertex, GL_STATIC_DRAW>;
}
