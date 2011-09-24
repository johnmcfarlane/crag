//
//  Cuboid.h
//  crag
//
//  Created by John McFarlane on 9/5/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "glpp/Mesh.h"
#include "glpp/Vbo_Types.h"


namespace gfx
{
	// Graphical representation of a sphere.
	// Has a mesh containing tesselations of a sphere at multiple levels of detail.
	class Cuboid
	{
		// types
		typedef Vector3f Vector3;
		typedef gl::Vertex3dNorm Vertex;
		typedef gl::Mesh<Vertex, gl::STATIC_DRAW> Mesh;
		typedef gl::VertexRgba Color;
		typedef std::vector<Vertex> VertexVector;
	public:
		
		// functions
		Cuboid();
		~Cuboid();
		
		void Draw() const;
	private:
		
		// variables
		Mesh _mesh;
	};
}
