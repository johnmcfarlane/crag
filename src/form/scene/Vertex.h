//
//  Vertex.h
//  crag
//
//  Created by John McFarlane on 2012-01-23.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "gfx/Color.h"


namespace form
{
	// Vertex type used by formation system to generate OpenGL mesh.
	struct Vertex
	{
		geom::Vector3f pos;
		geom::Vector3f norm;
		gfx::Color4b col;
	};
}
