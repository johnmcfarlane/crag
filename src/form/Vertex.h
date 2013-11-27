//
//  Vertex.h
//  crag
//
//  Created by John McFarlane on 2012-01-23.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "form/defs.h"
#include "gfx/Color.h"

namespace form
{
	// Vertex type used by formation system to generate OpenGL mesh.
	struct Vertex
	{
		CRAG_VERIFY_INVARIANTS_DECLARE(Vertex);

		Vector3 pos;
		Vector3 norm;
		gfx::Color4b col;
	};
}
