//
//  Quad.h
//  crag
//
//  Created by John McFarlane on 2012-01-01.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#pragma once

#include "defs.h"

#include "glpp/Vbo_Types.h"


namespace gfx
{
	// Graphical representation of a sphere.
	// Draws a quad in front of the sphere and has a specialized shader draw a sphere upon it.
	class Quad
	{
		// types
		typedef gl::Vertex3d QuadVertex;
		typedef gl::Vbo<QuadVertex> VertexBuffer;
	public:
		
		// functions
		Quad(Scalar depth_offset);
		~Quad();
		
		gfx::Transformation const & Transform(gfx::Transformation const & model_view, gfx::Transformation & scratch) const;

		void Draw() const;
		
	private:
		
		// variables
		VertexBuffer _quad;
	};
}