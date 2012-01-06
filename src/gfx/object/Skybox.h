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

#include "glpp/Vbo_Types.h"
#include "glpp/Texture.h"


namespace gfx
{
	class Image;
	class Scene;
	
	class Skybox : public LeafNode
	{
		// Types
		typedef gl::Vbo3dTex Vbo;
		typedef gl::TextureRgba8 Texture;
		
		// Methods
	public:
		Skybox();
		
		bool Init(Renderer const & renderer) override;
		void Deinit() override;
		
		void SetSide(int axis, int pole, Image const & image);
		
		gfx::Transformation const & Transform(gfx::Transformation const & model_view, gfx::Transformation & scratch, Time time) const override;
		void Render(Renderer const & renderer) const override;
		
	private:
		void InitVerts();
		
		// Attribs
		Texture sides[3][2];
		Vbo vbo;
	};
	
}
