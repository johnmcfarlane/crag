//
//  Sphere.h
//  crag
//
//  Created by John McFarlane on 7/9/11.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#pragma once

#include "defs.h"
#include "Mesh.h"

#include "glpp/Vbo_Types.h"


namespace gfx
{
	// Graphical representation of a sphere.
	// Has a mesh containing tesselations of a sphere at multiple levels of detail.
	class Sphere
	{
		// types
	public:
		typedef gl::Vertex3dNormHybrid Vertex;
	private:
		typedef gfx::Mesh<Vertex, gl::STATIC_DRAW> Mesh;
		typedef std::vector<Vertex> VertexVector;
	public:
		
		// constants
		static int const num_sphere_lods = 5;
		
		// functions
		Sphere();
		~Sphere();
		
		// Given a model transformation, calculate the radius at which the spherewould be drawn.
		static Scalar CalculateRadius(Transformation const & transformation);
		
		void Draw(Transformation const & transformation) const;
		void Draw(int lod = num_sphere_lods - 1) const;
	private:
		unsigned CalculateLod(Scalar radius, Scalar inv_distance_to_camera) const;
		
		// variables
		Mesh _mesh;
		std::vector<float> _lod_coefficients;
	};
}
