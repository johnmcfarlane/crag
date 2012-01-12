//
//  SphereQuad.h
//  crag
//
//  Created by John McFarlane on 2012-01-01.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#pragma once

#include "defs.h"

#include "object/Light.h"

#include "glpp/Vbo_Types.h"


namespace gfx
{
	// forward declarations
	class Program;

	// Graphical representation of a sphere.
	// Draws a quad in front of the sphere and has a specialized shader draw a sphere upon it.
	class SphereQuad
	{
		// types
	public:
		typedef gl::Vertex3d QuadVertex;
	private:
		typedef gl::Vbo<QuadVertex> Quad;
	public:
		
		// functions
		SphereQuad(Program & program);
		~SphereQuad();
		
		// Given a model transformation, calculate the radius at which the spherewould be drawn.
		static Scalar CalculateRadius(gfx::Transformation const & transformation);
		
		gfx::Transformation const & Transform(gfx::Transformation const & model_view, gfx::Transformation & scratch) const;

		void Draw(::Transformation<float> const & model_view) const;
		
	private:
		void InitProgram(Program & program);
		void InitQuad();
		
		// variables
		Quad _quad;
		
		GLint _quad_location;
		GLint _center_location;
		GLint _radius_location;
	};
}
