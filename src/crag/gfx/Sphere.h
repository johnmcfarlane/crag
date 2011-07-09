//
//  Sphere.h
//  crag
//
//  Created by John McFarlane on 7/9/11.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#pragma once

#include "glpp/Mesh.h"
#include "glpp/Vertex.h"


namespace gfx
{
	// Graphical representation of a sphere.
	// Has a mesh containing tesselations of a sphere at multiple levels of detail.
	class Sphere
	{
	public:
		// constants
		static int const num_sphere_lods = 5;
		
		// functions
		Sphere();
		~Sphere();
		
		void Draw(float radius, int lod = num_sphere_lods - 1) const;
		
		// types
		typedef gl::Vertex3dNorm Vertex;
	private:
		typedef gl::Mesh<Vertex> Mesh;
		
		// variables
		Mesh _mesh;
		std::vector<float> _lod_coefficients;
	};
}
