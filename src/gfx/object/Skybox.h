//
//  Skybox.h
//  crag
//
//  Created by John on 11/29/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "LeafNode.h"

#include "gfx/Texture.h"
#include "gfx/VertexBufferObject.h"


namespace gfx
{
	class Image;
	class Scene;
	
	class Skybox : public LeafNode
	{
	public:
		// Types
		struct Vertex
		{
			geom::Vector3f pos;
			geom::Vector2f tex;
		};
		
		typedef VertexBufferObject<Vertex> Vbo;
		
		// Methods
		Skybox();
		
		bool Init(gfx::Engine & renderer);
		void Deinit(Scene & scene) override;
		
		void SetSide(int axis, int pole, Image const & image);
		
		gfx::Transformation const & Transform(gfx::Engine & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const override;
		void Render(gfx::Engine const & renderer) const override;
		
	private:
		void InitVerts();
		
		// Attribs
		Texture sides[3][2];
		Vbo vbo;
	};
	
}
