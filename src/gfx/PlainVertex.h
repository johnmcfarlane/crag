//
//  gfx/PlainVertex.h
//  crag
//
//  Created by John McFarlane on 2014-01-08.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"
#include "Mesh.h"

namespace gfx
{
	struct PlainVertex
	{
		typedef ::gfx::Vector3 Vector3;
		
		CRAG_VERIFY_INVARIANTS_DECLARE(PlainVertex);

		Vector3 pos;
	};
	
	using PlainMesh = Mesh<PlainVertex, ElementIndex>;
	using PlainMeshHandle = crag::core::ResourceHandle<PlainMesh>;
}
