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
#include "VboResource.h"
#include "VertexBufferObject.h"


namespace gfx
{
	// Graphical representation of a sphere.
	// Draws a quad in front of the sphere and has a specialized shader draw a sphere upon it.
	class Quad : public VboResource
	{
	public:
		// types
		struct Vertex
		{
			geom::Vector3f pos;
		};

		typedef ::gfx::VertexBufferObject<Vertex> VertexBufferObject;
		
		// functions
		Quad(Quad && rhs);
		Quad(float depth_offset);
		
		Transformation CalculateModelViewTransformation(Transformation const & leaf, float radius) const;

		void Activate() const override;
		void Deactivate() const override;

		void Draw() const override;
		
	private:
		
		// variables
		VertexBufferObject _quad;
	};
}
