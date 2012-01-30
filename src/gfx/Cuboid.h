//
//  Cuboid.h
//  crag
//
//  Created by John McFarlane on 9/5/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Mesh.h"
#include "MeshResource.h"


namespace gfx
{
	// Graphical representation of a sphere.
	// Has a mesh containing tesselations of a sphere at multiple levels of detail.
	class Cuboid : public MeshResource
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
		
		void Activate() const override;
		void Deactivate() const override;
		
		void Draw() const;
	private:
		
		// variables
		Mesh _mesh;
	};
}
