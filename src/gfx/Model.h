//
//  Model.h
//  crag
//
//  Created by John McFarlane on 2014-01-08.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

namespace gfx
{
	template <typename Vertex>
	class Mesh;
	
	// a 3D shape stored in ResourceManager
	class Model
	{
	public:
		typedef Mesh<Vector3> ShadowVolumeMesh;
		
		virtual ~Model() = default;
	};
}
