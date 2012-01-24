//
//  Cuboid.h
//  crag
//
//  Created by John McFarlane on 9/5/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "gfx/Mesh.h"


namespace gfx
{
	// Graphical representation of a sphere.
	// Has a mesh containing tesselations of a sphere at multiple levels of detail.
	class Cuboid
	{
	public:
		// types
		struct Vertex
		{
			Vector3f pos;
			Vector3f norm;
		};
		
		typedef Vector3f Vector3;
		typedef gfx::Mesh<Vertex, GL_STATIC_DRAW> Mesh;
		
		// functions
		Cuboid();
		~Cuboid();
		
		void Draw() const;
	private:
		
		// variables
		Mesh _mesh;
	};
}
