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
		typedef gl::Mesh<Vertex, gl::STATIC_DRAW> Mesh;
		typedef gl::VertexRgba Color;
		typedef std::vector<Vertex> VertexVector;
	public:

		// constants
		static int const num_sphere_lods = 5;
		
		// functions
		Sphere();
		~Sphere();
		
		void Draw(float radius, int lod = num_sphere_lods - 1) const;
	private:
		void InitColors(VertexVector const & vertex_positions);
		void ActivateColors() const;
		void DeactivateColors() const;
	public:
		
		// variables
		Mesh _mesh;
		gl::VboRgba _color_vbo;
		std::vector<float> _lod_coefficients;
	};
}
